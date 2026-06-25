#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// ── Constructor ───────────────────────────────────────────────────────────────

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    CurrentZone = EAudio_AmbientZone::None;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    ScreenShakeIntensity = 1.0f;
    TRexShakeRadius = 3000.0f;
    ZoneTransitionSpeed = 1.0f;
    ThreatTransitionTimer = 0.0f;
    ZoneTransitionTimer = 0.0f;

    // Create main music audio component
    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->SetupAttachment(RootComponent);
    MusicComponent->bAutoActivate = false;
    MusicComponent->bIsUISound = false;
}

// ── BeginPlay ─────────────────────────────────────────────────────────────────

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Register default narrative lines from ElevenLabs TTS (generated this cycle)
    FAudio_NarrativeLine TRexLine;
    TRexLine.CharacterName = TEXT("Narrator");
    TRexLine.AudioURL = TTS_NarratorTRexProximity;
    TRexLine.SubtitleText = TEXT("Something is moving through the trees. Heavy. Slow. The ground shakes with every step. T-Rex. Do not move. Do not breathe. Wait for it to pass.");
    TRexLine.DisplayDuration = 10.0f;
    NarrativeLines.Add(TRexLine);

    FAudio_NarrativeLine WaterLine;
    WaterLine.CharacterName = TEXT("Scout Mira");
    WaterLine.AudioURL = TTS_ScoutMiraWater;
    WaterLine.SubtitleText = TEXT("Water. Fresh water, fifty paces east. Fill your skins now — the dry season comes fast in this valley. And watch the banks. Crocodiles nest near still pools.");
    WaterLine.DisplayDuration = 11.0f;
    NarrativeLines.Add(WaterLine);

    // Register default dino sound profiles (sounds assigned via Blueprint or data asset)
    FAudio_DinoSoundProfile TRexProfile;
    TRexProfile.DinoSpecies = TEXT("TRex");
    TRexProfile.RoarRadius = 6000.0f;
    TRexProfile.FootstepRadius = 3000.0f;
    DinoSoundProfiles.Add(TRexProfile);

    FAudio_DinoSoundProfile RaptorProfile;
    RaptorProfile.DinoSpecies = TEXT("Velociraptor");
    RaptorProfile.RoarRadius = 2500.0f;
    RaptorProfile.FootstepRadius = 800.0f;
    DinoSoundProfiles.Add(RaptorProfile);

    FAudio_DinoSoundProfile TrikeProfile;
    TrikeProfile.DinoSpecies = TEXT("Triceratops");
    TrikeProfile.RoarRadius = 3500.0f;
    TrikeProfile.FootstepRadius = 2000.0f;
    DinoSoundProfiles.Add(TrikeProfile);

    // Start in jungle ambient zone by default
    SetAmbientZone(EAudio_AmbientZone::Jungle);
}

// ── Tick ──────────────────────────────────────────────────────────────────────

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAmbientVolumes(DeltaTime);
    UpdateMusicTransition(DeltaTime);
}

// ── Ambient Zone System ───────────────────────────────────────────────────────

void AAudio_SystemManager::SetAmbientZone(EAudio_AmbientZone NewZone)
{
    if (CurrentZone == NewZone)
    {
        return;
    }

    CurrentZone = NewZone;
    ZoneTransitionTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Zone changed to %d"), (int32)NewZone);
}

void AAudio_SystemManager::UpdateAmbientVolumes(float DeltaTime)
{
    ZoneTransitionTimer += DeltaTime * ZoneTransitionSpeed;

    // Fade ambient components based on current zone
    for (int32 i = 0; i < AmbientComponents.Num(); i++)
    {
        if (!AmbientComponents.IsValidIndex(i) || !AmbientComponents[i])
        {
            continue;
        }

        UAudioComponent* Comp = AmbientComponents[i];
        if (!AmbientLayers.IsValidIndex(i))
        {
            continue;
        }

        const FAudio_AmbientLayer& Layer = AmbientLayers[i];
        float TargetVolume = (Layer.Zone == CurrentZone) ? Layer.BaseVolume : 0.0f;
        float CurrentVolume = Comp->VolumeMultiplier;
        float FadeSpeed = (TargetVolume > CurrentVolume) ? (1.0f / FMath::Max(Layer.FadeInTime, 0.01f)) : (1.0f / FMath::Max(Layer.FadeOutTime, 0.01f));
        float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, FadeSpeed);
        Comp->SetVolumeMultiplier(NewVolume);
    }
}

// ── Threat Level Music System ─────────────────────────────────────────────────

void AAudio_SystemManager::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (CurrentThreatLevel == NewLevel)
    {
        return;
    }

    CurrentThreatLevel = NewLevel;
    ThreatTransitionTimer = 0.0f;

    // Select music track based on threat level
    USoundBase* TargetMusic = nullptr;
    switch (NewLevel)
    {
        case EAudio_ThreatLevel::Safe:     TargetMusic = MusicSafe;     break;
        case EAudio_ThreatLevel::Aware:    TargetMusic = MusicAware;    break;
        case EAudio_ThreatLevel::Danger:   TargetMusic = MusicDanger;   break;
        case EAudio_ThreatLevel::Critical: TargetMusic = MusicCritical; break;
        default: break;
    }

    if (MusicComponent && TargetMusic)
    {
        MusicComponent->FadeOut(2.0f, 0.0f);
        MusicComponent->SetSound(TargetMusic);
        MusicComponent->FadeIn(2.0f, 1.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: ThreatLevel changed to %d"), (int32)NewLevel);
}

void AAudio_SystemManager::UpdateMusicTransition(float DeltaTime)
{
    ThreatTransitionTimer += DeltaTime;
}

// ── Narrative Audio ───────────────────────────────────────────────────────────

void AAudio_SystemManager::PlayNarrativeLine(int32 LineIndex)
{
    if (!NarrativeLines.IsValidIndex(LineIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: Invalid narrative line index %d"), LineIndex);
        return;
    }

    const FAudio_NarrativeLine& Line = NarrativeLines[LineIndex];

    // Play the sound asset if assigned
    if (Line.SoundAsset)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Line.SoundAsset, 1.0f, 1.0f, 0.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing narrative line %d — %s: %s"),
        LineIndex,
        *Line.CharacterName,
        *Line.SubtitleText);
}

void AAudio_SystemManager::AddNarrativeLine(const FAudio_NarrativeLine& Line)
{
    NarrativeLines.Add(Line);
}

// ── Dino Sound Profiles ───────────────────────────────────────────────────────

void AAudio_SystemManager::PlayDinoRoar(const FString& DinoSpecies, FVector Location)
{
    for (const FAudio_DinoSoundProfile& Profile : DinoSoundProfiles)
    {
        if (Profile.DinoSpecies == DinoSpecies && Profile.RoarSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                Profile.RoarSound,
                Location,
                1.0f,
                1.0f,
                0.0f
            );
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Dino roar — %s at %s"), *DinoSpecies, *Location.ToString());
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No roar sound for species %s"), *DinoSpecies);
}

void AAudio_SystemManager::PlayDinoFootstep(const FString& DinoSpecies, FVector Location)
{
    for (const FAudio_DinoSoundProfile& Profile : DinoSoundProfiles)
    {
        if (Profile.DinoSpecies == DinoSpecies && Profile.FootstepSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                Profile.FootstepSound,
                Location,
                1.0f,
                1.0f,
                0.0f
            );
            return;
        }
    }
}

void AAudio_SystemManager::RegisterDinoProfile(const FAudio_DinoSoundProfile& Profile)
{
    // Remove existing profile for this species if present
    DinoSoundProfiles.RemoveAll([&Profile](const FAudio_DinoSoundProfile& Existing)
    {
        return Existing.DinoSpecies == Profile.DinoSpecies;
    });
    DinoSoundProfiles.Add(Profile);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered dino sound profile for %s"), *Profile.DinoSpecies);
}

// ── Screen Feedback ───────────────────────────────────────────────────────────

void AAudio_SystemManager::TriggerTRexProximityShake(FVector TRexLocation)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        return;
    }

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), TRexLocation);
    if (Distance > TRexShakeRadius)
    {
        return;
    }

    // Scale shake intensity by proximity (closer = stronger)
    float ProximityFactor = 1.0f - (Distance / TRexShakeRadius);
    float FinalIntensity = ScreenShakeIntensity * ProximityFactor;

    // Footstep audio feedback — low frequency rumble cue
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: T-Rex proximity shake — distance=%.0f, intensity=%.2f"), Distance, FinalIntensity);

    // Elevate threat level when T-Rex is very close
    if (Distance < TRexShakeRadius * 0.3f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Critical);
    }
    else if (Distance < TRexShakeRadius * 0.6f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Danger);
    }
    else
    {
        SetThreatLevel(EAudio_ThreatLevel::Aware);
    }
}

void AAudio_SystemManager::TriggerDamageAudioFeedback(float DamageAmount)
{
    // Clamp damage to 0-100 range for intensity calculation
    float ClampedDamage = FMath::Clamp(DamageAmount, 0.0f, 100.0f);
    float FeedbackIntensity = ClampedDamage / 100.0f;

    // Escalate threat level on significant damage
    if (FeedbackIntensity > 0.5f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Critical);
    }
    else if (FeedbackIntensity > 0.25f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Danger);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Damage audio feedback — damage=%.1f, intensity=%.2f"), DamageAmount, FeedbackIntensity);
}
