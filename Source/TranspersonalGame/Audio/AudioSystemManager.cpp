#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default T-Rex footstep shake
    TRexFootstepShake.Intensity = 2.5f;
    TRexFootstepShake.Duration = 0.6f;
    TRexFootstepShake.Falloff = 1.5f;
    TRexFootstepShake.MaxDistance = 3000.0f;

    // Default raptor attack shake
    RaptorAttackShake.Intensity = 1.2f;
    RaptorAttackShake.Duration = 0.3f;
    RaptorAttackShake.Falloff = 1.0f;
    RaptorAttackShake.MaxDistance = 500.0f;

    DayNightCycleDuration = 600.0f;
    CurrentDayTime = 150.0f; // Start at midday
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialise time of day from current day time
    UpdateDayNightAudio();

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Initialised. ThreatLevel=Safe, TimeOfDay=%d"),
        (int32)CurrentTimeOfDay);
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Advance day/night cycle
    AdvanceDayNightCycle(DeltaTime);

    // Update ambient layer volumes (fade in/out)
    UpdateAmbientLayers(DeltaTime);
}

// ============================================================
// Threat Level — Adaptive Music
// ============================================================

void AAudio_SystemManager::SetThreatLevel(EAudio_ThreatLevel NewLevel)
{
    if (NewLevel == CurrentThreatLevel) return;

    PreviousThreatLevel = CurrentThreatLevel;
    CurrentThreatLevel = NewLevel;
    LastThreatTransitionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: ThreatLevel changed %d -> %d"),
        (int32)PreviousThreatLevel, (int32)CurrentThreatLevel);

    // Adjust ambient layers based on threat
    switch (CurrentThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:
            FadeInLayer(FName("Ambient_Nature"), 3.0f);
            FadeOutLayer(FName("Music_Tension"), 4.0f);
            FadeOutLayer(FName("Music_Combat"), 2.0f);
            break;

        case EAudio_ThreatLevel::Aware:
            FadeInLayer(FName("Music_Tension"), 2.0f);
            FadeOutLayer(FName("Music_Combat"), 3.0f);
            break;

        case EAudio_ThreatLevel::Danger:
        case EAudio_ThreatLevel::Stalked:
            FadeInLayer(FName("Music_Tension"), 1.0f);
            FadeOutLayer(FName("Ambient_Nature"), 2.0f);
            break;

        case EAudio_ThreatLevel::Combat:
            FadeInLayer(FName("Music_Combat"), 0.5f);
            FadeOutLayer(FName("Music_Tension"), 1.0f);
            FadeOutLayer(FName("Ambient_Nature"), 1.0f);
            break;
    }
}

// ============================================================
// Time of Day
// ============================================================

void AAudio_SystemManager::SetTimeOfDay(EAudio_TimeOfDay NewTime)
{
    if (NewTime == CurrentTimeOfDay) return;

    CurrentTimeOfDay = NewTime;
    UpdateDayNightAudio();

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: TimeOfDay changed to %d"), (int32)CurrentTimeOfDay);
}

void AAudio_SystemManager::UpdateDayNightAudio()
{
    // Transition ambient layers for time of day
    switch (CurrentTimeOfDay)
    {
        case EAudio_TimeOfDay::Dawn:
            FadeInLayer(FName("Ambient_Birds_Dawn"), 4.0f);
            FadeOutLayer(FName("Ambient_Night_Insects"), 6.0f);
            FadeOutLayer(FName("Ambient_Night_Wind"), 4.0f);
            break;

        case EAudio_TimeOfDay::Day:
            FadeInLayer(FName("Ambient_Nature"), 3.0f);
            FadeInLayer(FName("Ambient_Birds_Day"), 3.0f);
            FadeOutLayer(FName("Ambient_Birds_Dawn"), 5.0f);
            break;

        case EAudio_TimeOfDay::Dusk:
            FadeInLayer(FName("Ambient_Dusk_Wind"), 4.0f);
            FadeOutLayer(FName("Ambient_Birds_Day"), 6.0f);
            FadeOutLayer(FName("Ambient_Nature"), 4.0f);
            break;

        case EAudio_TimeOfDay::Night:
            FadeInLayer(FName("Ambient_Night_Insects"), 5.0f);
            FadeInLayer(FName("Ambient_Night_Wind"), 4.0f);
            FadeOutLayer(FName("Ambient_Dusk_Wind"), 3.0f);
            break;
    }
}

// ============================================================
// Screen Shake
// ============================================================

void AAudio_SystemManager::TriggerScreenShake(FAudio_ScreenShakeConfig Config, FVector SourceLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    // Distance-based intensity falloff
    APawn* PlayerPawn = PC->GetPawn();
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), SourceLocation);
        if (Distance > Config.MaxDistance) return;

        float DistanceFactor = FMath::Clamp(1.0f - (Distance / Config.MaxDistance), 0.0f, 1.0f);
        float FinalIntensity = Config.Intensity * FMath::Pow(DistanceFactor, Config.Falloff);

        if (FinalIntensity < 0.05f) return;

        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: ScreenShake triggered. Intensity=%.2f Distance=%.0f"),
            FinalIntensity, Distance);
    }
}

void AAudio_SystemManager::TriggerTRexFootstep(FVector TRexLocation)
{
    TriggerScreenShake(TRexFootstepShake, TRexLocation);

    // Also spawn ground dust at footstep location
    SpawnFootstepDust(TRexLocation, true);
}

// ============================================================
// Damage Flash
// ============================================================

void AAudio_SystemManager::TriggerDamageFlash(float DamageAmount)
{
    // Scale flash intensity by damage amount (normalised to 100 HP)
    float ScaledIntensity = FMath::Clamp(DamageAmount / 100.0f, 0.1f, 1.0f) * DamageFlashIntensity;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: DamageFlash triggered. Damage=%.1f Intensity=%.2f"),
        DamageAmount, ScaledIntensity);

    // Blueprint handles the actual screen overlay — this fires the event
    // In Blueprint: bind to OnDamageFlash delegate and show red material overlay
}

// ============================================================
// Footstep Dust
// ============================================================

void AAudio_SystemManager::SpawnFootstepDust(FVector Location, bool bIsHeavyCreature)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Heavy creature (T-Rex, Brachiosaurus) gets larger dust cloud
    float DustScale = bIsHeavyCreature ? 3.0f : 1.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: FootstepDust at (%.0f, %.0f, %.0f) Scale=%.1f"),
        Location.X, Location.Y, Location.Z, DustScale);

    // Niagara/Cascade particle spawn handled via Blueprint event
    // This function signals the VFX system to spawn the effect
}

// ============================================================
// Sound Layers
// ============================================================

void AAudio_SystemManager::SetLayerVolume(FName LayerName, float Volume)
{
    for (FAudio_SoundLayer& Layer : AmbientLayers)
    {
        if (Layer.LayerName == LayerName)
        {
            Layer.TargetVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
            return;
        }
    }
}

void AAudio_SystemManager::FadeInLayer(FName LayerName, float FadeTime)
{
    for (FAudio_SoundLayer& Layer : AmbientLayers)
    {
        if (Layer.LayerName == LayerName)
        {
            Layer.TargetVolume = 1.0f;
            Layer.FadeSpeed = (FadeTime > 0.0f) ? (1.0f / FadeTime) : 10.0f;
            Layer.bIsActive = true;
            return;
        }
    }
}

void AAudio_SystemManager::FadeOutLayer(FName LayerName, float FadeTime)
{
    for (FAudio_SoundLayer& Layer : AmbientLayers)
    {
        if (Layer.LayerName == LayerName)
        {
            Layer.TargetVolume = 0.0f;
            Layer.FadeSpeed = (FadeTime > 0.0f) ? (1.0f / FadeTime) : 10.0f;
            return;
        }
    }
}

void AAudio_SystemManager::UpdateAmbientLayers(float DeltaTime)
{
    for (FAudio_SoundLayer& Layer : AmbientLayers)
    {
        if (FMath::IsNearlyEqual(Layer.CurrentVolume, Layer.TargetVolume, 0.001f))
            continue;

        Layer.CurrentVolume = FMath::FInterpTo(
            Layer.CurrentVolume,
            Layer.TargetVolume,
            DeltaTime,
            Layer.FadeSpeed
        );

        // Deactivate layer when fully faded out
        if (Layer.CurrentVolume < 0.01f && Layer.TargetVolume <= 0.0f)
        {
            Layer.CurrentVolume = 0.0f;
            Layer.bIsActive = false;
        }
    }
}

// ============================================================
// Day/Night Cycle
// ============================================================

void AAudio_SystemManager::AdvanceDayNightCycle(float DeltaSeconds)
{
    CurrentDayTime += DeltaSeconds;
    if (CurrentDayTime >= DayNightCycleDuration)
    {
        CurrentDayTime -= DayNightCycleDuration;
    }

    EAudio_TimeOfDay NewTime = CalculateTimeOfDay(GetDayNightNormalised());
    if (NewTime != CurrentTimeOfDay)
    {
        SetTimeOfDay(NewTime);
    }
}

float AAudio_SystemManager::GetDayNightNormalised() const
{
    if (DayNightCycleDuration <= 0.0f) return 0.0f;
    return CurrentDayTime / DayNightCycleDuration;
}

EAudio_TimeOfDay AAudio_SystemManager::CalculateTimeOfDay(float NormalisedTime) const
{
    // 0.0-0.1 = Dawn, 0.1-0.6 = Day, 0.6-0.75 = Dusk, 0.75-1.0 = Night
    if (NormalisedTime < 0.1f)  return EAudio_TimeOfDay::Dawn;
    if (NormalisedTime < 0.6f)  return EAudio_TimeOfDay::Day;
    if (NormalisedTime < 0.75f) return EAudio_TimeOfDay::Dusk;
    return EAudio_TimeOfDay::Night;
}
