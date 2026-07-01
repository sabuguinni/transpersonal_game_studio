#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

// ============================================================
// Constructor
// ============================================================

UAudio_SystemManager::UAudio_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->VolumeMultiplier = 0.6f;

    // Initialize default narrator lines with ElevenLabs TTS URLs
    PopulateDefaultNarratorLines();
    PopulateDefaultDinoProfiles();
}

// ============================================================
// BeginPlay
// ============================================================

void UAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Reset state
    TimeSinceLastNarratorLine = NarratorCooldownSeconds; // Allow first line immediately
    CurrentMusicBlend = 0.0f;
    PreviousZone = CurrentAmbientZone;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Initialized. Zone=%d, Threat=%d, NarratorLines=%d, DinoProfiles=%d"),
        (int32)CurrentAmbientZone,
        (int32)CurrentThreatLevel,
        NarratorLines.Num(),
        DinoSoundProfiles.Num());
}

// ============================================================
// TickComponent
// ============================================================

void UAudio_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateMusicBlend(DeltaTime);
    UpdateNarratorCooldown(DeltaTime);
}

// ============================================================
// SetAmbientZone
// ============================================================

void UAudio_SystemManager::SetAmbientZone(EAudio_AmbientZone NewZone)
{
    if (NewZone == CurrentAmbientZone)
    {
        return;
    }

    PreviousZone = CurrentAmbientZone;
    CurrentAmbientZone = NewZone;
    CurrentMusicBlend = 0.0f; // Reset blend for crossfade

    UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Ambient zone changed: %d -> %d"),
        (int32)PreviousZone, (int32)CurrentAmbientZone);

    // Adjust ambient volume based on zone
    if (AmbientAudioComponent)
    {
        switch (CurrentAmbientZone)
        {
        case EAudio_AmbientZone::CaveEntrance:
            AmbientAudioComponent->VolumeMultiplier = 0.3f;
            break;
        case EAudio_AmbientZone::OpenPlains:
            AmbientAudioComponent->VolumeMultiplier = 0.8f;
            break;
        case EAudio_AmbientZone::DenseForest:
            AmbientAudioComponent->VolumeMultiplier = 1.0f;
            break;
        case EAudio_AmbientZone::NightTime:
            AmbientAudioComponent->VolumeMultiplier = 0.5f;
            break;
        default:
            AmbientAudioComponent->VolumeMultiplier = 0.6f;
            break;
        }
    }
}

// ============================================================
// SetThreatLevel
// ============================================================

void UAudio_SystemManager::SetThreatLevel(EAudio_ThreatLevel NewThreat)
{
    if (NewThreat == CurrentThreatLevel)
    {
        return;
    }

    EAudio_ThreatLevel OldThreat = CurrentThreatLevel;
    CurrentThreatLevel = NewThreat;

    // Map threat to music intensity target
    switch (CurrentThreatLevel)
    {
    case EAudio_ThreatLevel::Safe:       ThreatMusicIntensity = 0.0f; break;
    case EAudio_ThreatLevel::Aware:      ThreatMusicIntensity = 0.25f; break;
    case EAudio_ThreatLevel::Stalked:    ThreatMusicIntensity = 0.55f; break;
    case EAudio_ThreatLevel::Combat:     ThreatMusicIntensity = 0.85f; break;
    case EAudio_ThreatLevel::Fleeing:    ThreatMusicIntensity = 1.0f; break;
    }

    UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Threat level: %d -> %d (intensity=%.2f)"),
        (int32)OldThreat, (int32)CurrentThreatLevel, ThreatMusicIntensity);

    // Trigger narrator line on significant threat escalation
    if ((int32)NewThreat > (int32)OldThreat + 1)
    {
        TriggerNarratorLine(NewThreat);
    }
}

// ============================================================
// TriggerNarratorLine
// ============================================================

void UAudio_SystemManager::TriggerNarratorLine(EAudio_ThreatLevel ForThreat)
{
    // Cooldown guard
    if (bNarratorPlaying || TimeSinceLastNarratorLine < NarratorCooldownSeconds)
    {
        return;
    }

    // Find best matching line for this threat level
    FAudio_NarratorLine* BestLine = nullptr;
    for (FAudio_NarratorLine& Line : NarratorLines)
    {
        if (Line.TriggerThreatLevel == ForThreat)
        {
            BestLine = &Line;
            break;
        }
    }

    if (!BestLine)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AudioSystem] No narrator line for threat level %d"), (int32)ForThreat);
        return;
    }

    // Log the audio URL — in production this would be loaded as SoundWave
    UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Playing narrator: %s | URL: %s | Duration: %.1fs"),
        *BestLine->CharacterName,
        *BestLine->AudioURL,
        BestLine->EstimatedDurationSeconds);

    bNarratorPlaying = true;
    TimeSinceLastNarratorLine = 0.0f;

    // Schedule narrator end (simulated via timer)
    FTimerHandle NarratorTimer;
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            NarratorTimer,
            [this]() { bNarratorPlaying = false; },
            BestLine->EstimatedDurationSeconds,
            false
        );
    }
}

// ============================================================
// TriggerDinoFootstepShake
// ============================================================

void UAudio_SystemManager::TriggerDinoFootstepShake(EAudio_DinoSpecies Species, float DistanceMeters)
{
    FAudio_DinoSoundProfile Profile = GetDinoProfile(Species);

    // Scale shake intensity by distance (inverse square falloff)
    float MaxShakeDistance = Profile.FootstepRadius / 100.0f; // cm to meters
    if (DistanceMeters > MaxShakeDistance)
    {
        return; // Too far to feel
    }

    float DistanceFactor = FMath::Clamp(1.0f - (DistanceMeters / MaxShakeDistance), 0.0f, 1.0f);
    float ShakeStrength = Profile.GroundShakeIntensity * DistanceFactor;

    UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Dino footstep shake: Species=%d, Dist=%.1fm, Strength=%.2f"),
        (int32)Species, DistanceMeters, ShakeStrength);

    // Apply camera shake via PlayerController
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Use built-in camera shake — intensity drives shake amplitude
    // In production, reference a UCameraShakeBase subclass asset
    if (ShakeStrength > 0.1f)
    {
        UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Camera shake triggered: strength=%.2f"), ShakeStrength);
        // PC->ClientStartCameraShake(DinoFootstepShakeClass, ShakeStrength);
        // Placeholder: log the shake event for Blueprint to pick up
    }
}

// ============================================================
// GetDinoProfile
// ============================================================

FAudio_DinoSoundProfile UAudio_SystemManager::GetDinoProfile(EAudio_DinoSpecies Species) const
{
    for (const FAudio_DinoSoundProfile& Profile : DinoSoundProfiles)
    {
        if (Profile.Species == Species)
        {
            return Profile;
        }
    }

    // Return default profile if not found
    FAudio_DinoSoundProfile Default;
    Default.Species = Species;
    return Default;
}

// ============================================================
// PopulateDefaultNarratorLines (ElevenLabs TTS URLs from cycles)
// ============================================================

void UAudio_SystemManager::PopulateDefaultNarratorLines()
{
    NarratorLines.Empty();

    // From PROD_CYCLE_AUTO_20260701_010 — Narrator_TRex
    {
        FAudio_NarratorLine Line;
        Line.CharacterName = TEXT("Narrator_TRex");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782912128787_Narrator_TRex.mp3");
        Line.TranscriptText = TEXT("The T-Rex is close. I can hear its footsteps — each one shakes the ground beneath me. Do not move. Do not breathe.");
        Line.EstimatedDurationSeconds = 12.0f;
        Line.TriggerThreatLevel = EAudio_ThreatLevel::Stalked;
        NarratorLines.Add(Line);
    }

    // From PROD_CYCLE_AUTO_20260701_009 — Narrator_Night
    {
        FAudio_NarratorLine Line;
        Line.CharacterName = TEXT("Narrator_Night");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782912149000_Narrator_Night.mp3");
        Line.TranscriptText = TEXT("Night falls. The predators that sleep in daylight are waking now.");
        Line.EstimatedDurationSeconds = 8.0f;
        Line.TriggerThreatLevel = EAudio_ThreatLevel::Aware;
        NarratorLines.Add(Line);
    }

    // From PROD_CYCLE_AUTO_20260701_008 — Narrator_Danger
    {
        FAudio_NarratorLine Line;
        Line.CharacterName = TEXT("Narrator_Danger");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782907215913_Narrator_Danger.mp3");
        Line.TranscriptText = TEXT("Danger. Something large is moving through the trees. Stay low. Do not run.");
        Line.EstimatedDurationSeconds = 10.0f;
        Line.TriggerThreatLevel = EAudio_ThreatLevel::Combat;
        NarratorLines.Add(Line);
    }

    // From THIS CYCLE — Narrator_Silence
    {
        FAudio_NarratorLine Line;
        Line.CharacterName = TEXT("Narrator_Silence");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782921694481_Narrator_Silence.mp3");
        Line.TranscriptText = TEXT("The valley is silent. That is the warning. When the birds stop calling — something is hunting.");
        Line.EstimatedDurationSeconds = 13.0f;
        Line.TriggerThreatLevel = EAudio_ThreatLevel::Aware;
        NarratorLines.Add(Line);
    }

    // From THIS CYCLE — Narrator_Fire
    {
        FAudio_NarratorLine Line;
        Line.CharacterName = TEXT("Narrator_Fire");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782921715500_Narrator_Fire.mp3");
        Line.TranscriptText = TEXT("Fire. Keep it small. Enough to see your hands, not enough to be seen.");
        Line.EstimatedDurationSeconds = 14.0f;
        Line.TriggerThreatLevel = EAudio_ThreatLevel::Safe;
        NarratorLines.Add(Line);
    }

    UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Loaded %d narrator lines"), NarratorLines.Num());
}

// ============================================================
// PopulateDefaultDinoProfiles
// ============================================================

void UAudio_SystemManager::PopulateDefaultDinoProfiles()
{
    DinoSoundProfiles.Empty();

    // T-Rex — massive ground shake, long roar radius
    {
        FAudio_DinoSoundProfile Profile;
        Profile.Species = EAudio_DinoSpecies::TyrannosaurusRex;
        Profile.RoarRadius = 5000.0f;
        Profile.FootstepRadius = 2500.0f;
        Profile.FootstepInterval = 1.8f;
        Profile.GroundShakeIntensity = 2.0f;
        Profile.FreesoundQuery = TEXT("tyrannosaurus rex roar footstep heavy dinosaur");
        DinoSoundProfiles.Add(Profile);
    }

    // Raptor Pack — fast clicks, short radius, high frequency
    {
        FAudio_DinoSoundProfile Profile;
        Profile.Species = EAudio_DinoSpecies::VelociraptorPack;
        Profile.RoarRadius = 1500.0f;
        Profile.FootstepRadius = 600.0f;
        Profile.FootstepInterval = 0.4f;
        Profile.GroundShakeIntensity = 0.3f;
        Profile.FreesoundQuery = TEXT("raptor click hiss predator dinosaur");
        DinoSoundProfiles.Add(Profile);
    }

    // Triceratops — medium ground shake, herd rumble
    {
        FAudio_DinoSoundProfile Profile;
        Profile.Species = EAudio_DinoSpecies::Triceratops;
        Profile.RoarRadius = 2000.0f;
        Profile.FootstepRadius = 1200.0f;
        Profile.FootstepInterval = 1.1f;
        Profile.GroundShakeIntensity = 1.2f;
        Profile.FreesoundQuery = TEXT("large animal herd stampede rumble ground");
        DinoSoundProfiles.Add(Profile);
    }

    // Brachiosaurus — deep low rumble, very large radius
    {
        FAudio_DinoSoundProfile Profile;
        Profile.Species = EAudio_DinoSpecies::Brachiosaurus;
        Profile.RoarRadius = 4000.0f;
        Profile.FootstepRadius = 3000.0f;
        Profile.FootstepInterval = 2.5f;
        Profile.GroundShakeIntensity = 1.8f;
        Profile.FreesoundQuery = TEXT("elephant footstep heavy ground vibration low rumble");
        DinoSoundProfiles.Add(Profile);
    }

    // Pterodactyl — screech, no ground shake
    {
        FAudio_DinoSoundProfile Profile;
        Profile.Species = EAudio_DinoSpecies::Pterodactyl;
        Profile.RoarRadius = 2500.0f;
        Profile.FootstepRadius = 0.0f;
        Profile.FootstepInterval = 0.0f;
        Profile.GroundShakeIntensity = 0.0f;
        Profile.FreesoundQuery = TEXT("bird screech hawk eagle dive swoosh");
        DinoSoundProfiles.Add(Profile);
    }

    UE_LOG(LogTemp, Log, TEXT("[AudioSystem] Loaded %d dino sound profiles"), DinoSoundProfiles.Num());
}

// ============================================================
// Private helpers
// ============================================================

void UAudio_SystemManager::UpdateMusicBlend(float DeltaTime)
{
    // Smoothly blend music intensity toward target
    float TargetBlend = ThreatMusicIntensity;
    CurrentMusicBlend = FMath::FInterpTo(CurrentMusicBlend, TargetBlend, DeltaTime, AmbientTransitionSpeed);
}

void UAudio_SystemManager::UpdateNarratorCooldown(float DeltaTime)
{
    if (!bNarratorPlaying)
    {
        TimeSinceLastNarratorLine += DeltaTime;
    }
}
