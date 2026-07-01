#include "AudioSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraShakeBase.h"

// ============================================================
// Constructor
// ============================================================

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 Hz — audio state updates

    // Create audio components
    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->bAutoActivate = false;
    MusicComponent->VolumeMultiplier = MusicVolume;

    AmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientComponent"));
    AmbientComponent->bAutoActivate = false;
    AmbientComponent->VolumeMultiplier = AmbientVolume;

    NarrationComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationComponent"));
    NarrationComponent->bAutoActivate = false;
    NarrationComponent->VolumeMultiplier = NarrationVolume;

    // Default music state
    CurrentMusicState.ThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentMusicState.MusicIntensity = 0.0f;
    CurrentMusicState.TransitionSpeed = 1.5f;
    CurrentMusicState.bCombatActive = false;
    CurrentMusicState.bNightTime = false;

    // Register default dinosaur SFX profiles
    FAudio_DinosaurSFX TRexSFX;
    TRexSFX.DinosaurSpecies = FName("TRex");
    TRexSFX.RoarRadius = 5000.0f;
    TRexSFX.FootstepRadius = 2500.0f;
    TRexSFX.BreathRadius = 600.0f;
    TRexSFX.bCausesScreenShake = true;
    TRexSFX.ScreenShakeIntensity = 2.5f;
    RegisteredDinosaurSFX.Add(TRexSFX);

    FAudio_DinosaurSFX RaptorSFX;
    RaptorSFX.DinosaurSpecies = FName("Raptor");
    RaptorSFX.RoarRadius = 2000.0f;
    RaptorSFX.FootstepRadius = 800.0f;
    RaptorSFX.BreathRadius = 300.0f;
    RaptorSFX.bCausesScreenShake = false;
    RaptorSFX.ScreenShakeIntensity = 0.5f;
    RegisteredDinosaurSFX.Add(RaptorSFX);

    FAudio_DinosaurSFX BrachioSFX;
    BrachioSFX.DinosaurSpecies = FName("Brachiosaurus");
    BrachioSFX.RoarRadius = 4000.0f;
    BrachioSFX.FootstepRadius = 3000.0f;
    BrachioSFX.BreathRadius = 800.0f;
    BrachioSFX.bCausesScreenShake = true;
    BrachioSFX.ScreenShakeIntensity = 1.8f;
    RegisteredDinosaurSFX.Add(BrachioSFX);

    // Register default ambient layers
    FAudio_AmbientLayer PlainsDay;
    PlainsDay.BiomeZone = EAudio_BiomeZone::OpenPlains;
    PlainsDay.TimeOfDay = EAudio_TimeOfDay::Day;
    PlainsDay.BaseVolume = 0.8f;
    PlainsDay.FadeInTime = 3.0f;
    PlainsDay.FadeOutTime = 4.0f;
    AmbientLayers.Add(PlainsDay);

    FAudio_AmbientLayer ForestNight;
    ForestNight.BiomeZone = EAudio_BiomeZone::DenseForest;
    ForestNight.TimeOfDay = EAudio_TimeOfDay::Night;
    ForestNight.BaseVolume = 1.0f;
    ForestNight.FadeInTime = 5.0f;
    ForestNight.FadeOutTime = 6.0f;
    AmbientLayers.Add(ForestNight);

    FAudio_AmbientLayer CaveAmbient;
    CaveAmbient.BiomeZone = EAudio_BiomeZone::Cave;
    CaveAmbient.TimeOfDay = EAudio_TimeOfDay::Night;
    CaveAmbient.BaseVolume = 0.6f;
    CaveAmbient.FadeInTime = 2.0f;
    CaveAmbient.FadeOutTime = 2.0f;
    AmbientLayers.Add(CaveAmbient);
}

// ============================================================
// BeginPlay
// ============================================================

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: BeginPlay — initialising adaptive audio system"));
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: %d dinosaur SFX profiles registered"), RegisteredDinosaurSFX.Num());
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: %d ambient layers registered"), AmbientLayers.Num());
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Danger narration URL = %s"), *NarrationDanger_URL);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Survival narration URL = %s"), *NarrationSurvival_URL);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Campfire Freesound ID = %s"), *CampfireFreesoundID);
}

// ============================================================
// Tick
// ============================================================

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateMusicIntensity(DeltaTime);
    CrossfadeAmbientLayers(DeltaTime);

    // Narration queue timer
    if (bNarrationQueued)
    {
        NarrationQueueTimer -= DeltaTime;
        if (NarrationQueueTimer <= 0.0f)
        {
            bNarrationQueued = false;
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing queued narration: %s"), *QueuedNarrationURL);
        }
    }
}

// ============================================================
// Threat / Music
// ============================================================

void AAudio_SystemManager::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (CurrentMusicState.ThreatLevel == NewLevel) return;

    EAudio_ThreatLevel OldLevel = CurrentMusicState.ThreatLevel;
    CurrentMusicState.ThreatLevel = NewLevel;

    // Adjust transition speed based on urgency
    switch (NewLevel)
    {
        case EAudio_ThreatLevel::Safe:
            CurrentMusicState.TransitionSpeed = 0.5f; // slow fade to calm
            break;
        case EAudio_ThreatLevel::Cautious:
            CurrentMusicState.TransitionSpeed = 1.0f;
            break;
        case EAudio_ThreatLevel::Danger:
            CurrentMusicState.TransitionSpeed = 2.5f; // fast ramp up
            break;
        case EAudio_ThreatLevel::Critical:
            CurrentMusicState.TransitionSpeed = 5.0f; // immediate
            CurrentMusicState.bCombatActive = true;
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Threat level changed %d -> %d (speed=%.1f)"),
        (int32)OldLevel, (int32)NewLevel, CurrentMusicState.TransitionSpeed);
}

void AAudio_SystemManager::UpdateMusicIntensity(float DeltaTime)
{
    float TargetIntensity = 0.0f;
    switch (CurrentMusicState.ThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:     TargetIntensity = 0.0f;  break;
        case EAudio_ThreatLevel::Cautious: TargetIntensity = 0.35f; break;
        case EAudio_ThreatLevel::Danger:   TargetIntensity = 0.70f; break;
        case EAudio_ThreatLevel::Critical: TargetIntensity = 1.0f;  break;
    }

    // Night multiplier — everything feels more dangerous at night
    if (CurrentMusicState.bNightTime)
    {
        TargetIntensity = FMath::Min(TargetIntensity + 0.15f, 1.0f);
    }

    float InterpSpeed = CurrentMusicState.TransitionSpeed * DeltaTime;
    CurrentMusicState.MusicIntensity = FMath::FInterpTo(
        CurrentMusicState.MusicIntensity, TargetIntensity, DeltaTime, InterpSpeed);

    if (MusicComponent && MusicComponent->IsActive())
    {
        MusicComponent->SetVolumeMultiplier(CurrentMusicState.MusicIntensity * MusicVolume * MasterVolume);
    }
}

// ============================================================
// Ambient
// ============================================================

void AAudio_SystemManager::SetBiomeZone(EAudio_BiomeZone NewZone)
{
    if (CurrentBiome == NewZone) return;
    CurrentBiome = NewZone;
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Biome zone changed to %d"), (int32)NewZone);
}

void AAudio_SystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (CurrentTimeOfDay == NewTime) return;
    CurrentTimeOfDay = NewTime;
    CurrentMusicState.bNightTime = (NewTime == EAudio_TimeOfDay::Night);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Time of day changed to %d (night=%d)"),
        (int32)NewTime, CurrentMusicState.bNightTime ? 1 : 0);
}

void AAudio_SystemManager::CrossfadeAmbientLayers(float DeltaTime)
{
    // Find the matching ambient layer for current biome + time
    for (const FAudio_AmbientLayer& Layer : AmbientLayers)
    {
        if (Layer.BiomeZone == CurrentBiome && Layer.TimeOfDay == CurrentTimeOfDay)
        {
            float TargetVolume = Layer.BaseVolume * AmbientVolume * MasterVolume;
            if (AmbientComponent && AmbientComponent->IsActive())
            {
                float CurrentVol = AmbientComponent->VolumeMultiplier;
                float NewVol = FMath::FInterpTo(CurrentVol, TargetVolume, DeltaTime, 1.0f / Layer.FadeInTime);
                AmbientComponent->SetVolumeMultiplier(NewVol);
            }
            break;
        }
    }
}

// ============================================================
// Dinosaur SFX
// ============================================================

void AAudio_SystemManager::RegisterDinosaurSFX(const FAudio_DinosaurSFX& SFXConfig)
{
    // Remove existing entry for this species if present
    RegisteredDinosaurSFX.RemoveAll([&SFXConfig](const FAudio_DinosaurSFX& Existing)
    {
        return Existing.DinosaurSpecies == SFXConfig.DinosaurSpecies;
    });
    RegisteredDinosaurSFX.Add(SFXConfig);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Registered SFX for species '%s'"),
        *SFXConfig.DinosaurSpecies.ToString());
}

void AAudio_SystemManager::TriggerDinosaurRoar(FName Species, FVector Location)
{
    for (const FAudio_DinosaurSFX& SFX : RegisteredDinosaurSFX)
    {
        if (SFX.DinosaurSpecies == Species)
        {
            UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Roar triggered — %s at (%.0f, %.0f, %.0f) radius=%.0f"),
                *Species.ToString(), Location.X, Location.Y, Location.Z, SFX.RoarRadius);

            if (SFX.bCausesScreenShake)
            {
                TriggerScreenShake(SFX.ScreenShakeIntensity, Location);
            }

            // Escalate threat level if player is within roar radius
            APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
            if (PC && PC->GetPawn())
            {
                float DistToPlayer = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
                if (DistToPlayer < SFX.RoarRadius)
                {
                    float NormDist = 1.0f - FMath::Clamp(DistToPlayer / SFX.RoarRadius, 0.0f, 1.0f);
                    if (NormDist > 0.8f)
                        SetThreatLevel(EAudio_ThreatLevel::Critical);
                    else if (NormDist > 0.5f)
                        SetThreatLevel(EAudio_ThreatLevel::Danger);
                    else
                        SetThreatLevel(EAudio_ThreatLevel::Cautious);
                }
            }
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("AudioSystemManager: No SFX profile for species '%s'"), *Species.ToString());
}

void AAudio_SystemManager::TriggerDinosaurFootstep(FName Species, FVector Location, float Weight)
{
    for (const FAudio_DinosaurSFX& SFX : RegisteredDinosaurSFX)
    {
        if (SFX.DinosaurSpecies == Species)
        {
            if (SFX.bCausesScreenShake && Weight > 1000.0f)
            {
                float ShakeScale = FMath::Clamp(Weight / 5000.0f, 0.1f, 1.0f) * SFX.ScreenShakeIntensity * 0.3f;
                TriggerScreenShake(ShakeScale, Location);
            }
            return;
        }
    }
}

// ============================================================
// Screen Shake
// ============================================================

void AAudio_SystemManager::TriggerScreenShake(float Intensity, FVector SourceLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Distance-based intensity falloff
    if (PC->GetPawn())
    {
        float Dist = FVector::Dist(SourceLocation, PC->GetPawn()->GetActorLocation());
        float MaxDist = 4000.0f;
        float DistFactor = FMath::Clamp(1.0f - (Dist / MaxDist), 0.0f, 1.0f);
        Intensity *= DistFactor;
    }

    if (Intensity > 0.05f)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Screen shake intensity=%.2f from (%.0f, %.0f, %.0f)"),
            Intensity, SourceLocation.X, SourceLocation.Y, SourceLocation.Z);
        // PC->ClientStartCameraShake(ShakeClass, Intensity); // Wire up when CameraShake BP is ready
    }
}

// ============================================================
// Survival SFX
// ============================================================

void AAudio_SystemManager::PlayCraftingSound(FName ItemCrafted)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Crafting sound — item '%s'"), *ItemCrafted.ToString());
    // Stone tools: flint-knapping percussive sound
    // Wood tools: hollow wood knock
    // Fire: tinder crackle (Freesound ID 157187)
}

void AAudio_SystemManager::PlayFireSound(FVector FireLocation)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Fire sound at (%.0f, %.0f, %.0f) — Freesound ID %s"),
        FireLocation.X, FireLocation.Y, FireLocation.Z, *CampfireFreesoundID);
    // Freesound 157187: Campfire Crackle 3 — looping ambient at fire location
}

void AAudio_SystemManager::PlayDamageFlash(float DamageAmount)
{
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Damage flash — amount=%.1f"), DamageAmount);
    // Triggers both audio (impact grunt) and visual (red screen overlay via PostProcess)
    // Intensity scales with damage amount: 0-25 = minor, 25-75 = medium, 75+ = critical
    if (DamageAmount >= 75.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Critical);
    }
    else if (DamageAmount >= 25.0f)
    {
        SetThreatLevel(EAudio_ThreatLevel::Danger);
    }
}

// ============================================================
// Narration
// ============================================================

void AAudio_SystemManager::QueueNarrationLine(const FString& AudioURL, float Delay)
{
    QueuedNarrationURL = AudioURL;
    NarrationQueueTimer = Delay;
    bNarrationQueued = true;
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Narration queued (delay=%.1fs): %s"), Delay, *AudioURL);
}

void AAudio_SystemManager::StopNarration()
{
    bNarrationQueued = false;
    QueuedNarrationURL = TEXT("");
    if (NarrationComponent && NarrationComponent->IsActive())
    {
        NarrationComponent->Stop();
    }
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Narration stopped"));
}
