#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UAudio_SystemManager::UAudio_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — audio doesn't need per-frame update

    // Defaults
    CurrentZoneConfig.BiomeZone = EAudio_BiomeZone::OpenPlains;
    CurrentZoneConfig.AmbientVolume = 0.8f;
    CurrentZoneConfig.ReverbAmount = 0.1f;
    CurrentZoneConfig.MusicIntensity = 0.3f;
    CurrentZoneConfig.bNocturnalSounds = false;

    CurrentThreatState.ThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentThreatState.ThreatProximity = 2000.0f;
    CurrentThreatState.PredatorSpecies = TEXT("None");
    CurrentThreatState.MusicTransitionSpeed = 2.0f;
}

void UAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize audio state based on starting zone
    SetBiomeZone(EAudio_BiomeZone::OpenPlains);
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: BeginPlay — zone=OpenPlains, threat=Safe"));
}

void UAudio_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateMusicIntensity(DeltaTime);
    UpdateAmbientLayers(DeltaTime);
}

void UAudio_SystemManager::SetBiomeZone(EAudio_BiomeZone NewZone)
{
    if (CurrentZoneConfig.BiomeZone == NewZone)
    {
        return; // No change needed
    }

    CurrentZoneConfig.BiomeZone = NewZone;

    // Configure zone-specific audio parameters
    switch (NewZone)
    {
        case EAudio_BiomeZone::OpenPlains:
            CurrentZoneConfig.AmbientVolume = 0.6f;
            CurrentZoneConfig.ReverbAmount = 0.05f;
            CurrentZoneConfig.MusicIntensity = 0.3f;
            break;

        case EAudio_BiomeZone::DenseForest:
            CurrentZoneConfig.AmbientVolume = 1.0f;
            CurrentZoneConfig.ReverbAmount = 0.3f;
            CurrentZoneConfig.MusicIntensity = 0.5f;
            break;

        case EAudio_BiomeZone::RiverBank:
            CurrentZoneConfig.AmbientVolume = 0.9f;
            CurrentZoneConfig.ReverbAmount = 0.15f;
            CurrentZoneConfig.MusicIntensity = 0.35f;
            break;

        case EAudio_BiomeZone::CaveEntrance:
            CurrentZoneConfig.AmbientVolume = 0.4f;
            CurrentZoneConfig.ReverbAmount = 0.8f;
            CurrentZoneConfig.MusicIntensity = 0.6f;
            break;

        case EAudio_BiomeZone::VolcanicField:
            CurrentZoneConfig.AmbientVolume = 0.7f;
            CurrentZoneConfig.ReverbAmount = 0.2f;
            CurrentZoneConfig.MusicIntensity = 0.7f;
            break;

        case EAudio_BiomeZone::NightCampfire:
            CurrentZoneConfig.AmbientVolume = 0.5f;
            CurrentZoneConfig.ReverbAmount = 0.1f;
            CurrentZoneConfig.MusicIntensity = 0.2f;
            CurrentZoneConfig.bNocturnalSounds = true;
            break;

        default:
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Zone changed to %d — ambient=%.2f, reverb=%.2f, music=%.2f"),
        (int32)NewZone,
        CurrentZoneConfig.AmbientVolume,
        CurrentZoneConfig.ReverbAmount,
        CurrentZoneConfig.MusicIntensity);
}

void UAudio_SystemManager::UpdateThreatLevel(EAudio_ThreatLevel NewThreat, float Proximity, const FString& Species)
{
    if (CurrentThreatState.ThreatLevel == NewThreat &&
        CurrentThreatState.PredatorSpecies == Species)
    {
        // Update proximity only
        CurrentThreatState.ThreatProximity = Proximity;
        return;
    }

    EAudio_ThreatLevel PreviousThreat = CurrentThreatState.ThreatLevel;
    CurrentThreatState.ThreatLevel = NewThreat;
    CurrentThreatState.ThreatProximity = Proximity;
    CurrentThreatState.PredatorSpecies = Species;

    // Reset transition alpha for smooth music crossfade
    ThreatTransitionAlpha = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Threat changed %d→%d, species=%s, proximity=%.0f"),
        (int32)PreviousThreat,
        (int32)NewThreat,
        *Species,
        Proximity);

    // Trigger immediate audio feedback for escalating threats
    if (NewThreat == EAudio_ThreatLevel::Critical)
    {
        TriggerFootstepRumble(1.0f, 0.5f);
    }
    else if (NewThreat == EAudio_ThreatLevel::Danger)
    {
        TriggerFootstepRumble(0.6f, 0.3f);
    }
}

void UAudio_SystemManager::SetTimeOfDay(float NormalizedTime)
{
    // NormalizedTime: 0.0 = midnight, 0.25 = dawn, 0.5 = noon, 0.75 = dusk, 1.0 = midnight
    CurrentTimeOfDay = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);

    // Toggle nocturnal sounds based on time
    bool bIsNight = (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f);
    if (CurrentZoneConfig.bNocturnalSounds != bIsNight)
    {
        CurrentZoneConfig.bNocturnalSounds = bIsNight;
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Nocturnal sounds %s (time=%.2f)"),
            bIsNight ? TEXT("ON") : TEXT("OFF"),
            CurrentTimeOfDay);
    }
}

void UAudio_SystemManager::TriggerFootstepRumble(float Intensity, float Duration)
{
    if (!GetWorld())
    {
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        return;
    }

    // Apply camera shake via player controller
    // Intensity drives the shake magnitude — T-Rex footsteps = 1.0, Raptor = 0.4
    float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    float ClampedDuration = FMath::Clamp(Duration, 0.05f, 2.0f);

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: FootstepRumble — intensity=%.2f, duration=%.2fs"),
        ClampedIntensity,
        ClampedDuration);

    // Screen shake feedback — uses built-in UE5 controller rumble
    PC->PlayDynamicForceFeedback(
        ClampedIntensity,       // LeftIntensity
        ClampedDuration,        // Duration
        true,                   // bAffectsLeftLarge
        false,                  // bAffectsLeftSmall
        ClampedIntensity * 0.5f, // RightIntensity
        false,                  // bAffectsRightLarge
        false                   // bAffectsRightSmall
    );
}

void UAudio_SystemManager::SetCampfireActive(bool bActive)
{
    if (bCampfireActive == bActive)
    {
        return;
    }

    bCampfireActive = bActive;

    if (bActive)
    {
        SetBiomeZone(EAudio_BiomeZone::NightCampfire);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Campfire ACTIVE — switching to NightCampfire zone"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Campfire INACTIVE"));
    }
}

void UAudio_SystemManager::UpdateMusicIntensity(float DeltaTime)
{
    float TargetIntensity = GetMusicTargetIntensity();
    float TransitionSpeed = CurrentThreatState.MusicTransitionSpeed;

    // Smooth interpolation toward target music intensity
    ThreatTransitionAlpha = FMath::FInterpTo(
        ThreatTransitionAlpha,
        TargetIntensity,
        DeltaTime,
        TransitionSpeed
    );
}

void UAudio_SystemManager::UpdateAmbientLayers(float DeltaTime)
{
    // Time-of-day modulation of ambient volume
    // Dawn/dusk = peak bird activity, midday = reduced, night = insects
    float TimeModulation = 1.0f;

    if (CurrentTimeOfDay >= 0.2f && CurrentTimeOfDay <= 0.3f)
    {
        // Dawn — birds peak
        TimeModulation = 1.3f;
    }
    else if (CurrentTimeOfDay >= 0.7f && CurrentTimeOfDay <= 0.8f)
    {
        // Dusk — birds peak again
        TimeModulation = 1.2f;
    }
    else if (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f)
    {
        // Night — insects, reduced overall
        TimeModulation = 0.7f;
    }

    // Apply modulation to ambient volume (clamped)
    float ModulatedVolume = FMath::Clamp(
        CurrentZoneConfig.AmbientVolume * TimeModulation * MasterVolume,
        0.0f,
        1.0f
    );

    // In a full implementation, this would drive the MetaSound parameter
    // For now, log the computed value for debugging
    (void)ModulatedVolume; // Suppress unused warning until MetaSound integration
}

float UAudio_SystemManager::GetMusicTargetIntensity() const
{
    // Base intensity from biome zone
    float BaseIntensity = CurrentZoneConfig.MusicIntensity;

    // Threat multiplier
    float ThreatMultiplier = 1.0f;
    switch (CurrentThreatState.ThreatLevel)
    {
        case EAudio_ThreatLevel::Safe:
            ThreatMultiplier = 1.0f;
            break;
        case EAudio_ThreatLevel::Aware:
            ThreatMultiplier = 1.5f;
            break;
        case EAudio_ThreatLevel::Danger:
            ThreatMultiplier = 2.5f;
            break;
        case EAudio_ThreatLevel::Critical:
            ThreatMultiplier = 4.0f;
            break;
        default:
            ThreatMultiplier = 1.0f;
            break;
    }

    // Proximity modulation — closer predator = louder music
    float ProximityFactor = 1.0f;
    if (CurrentThreatState.ThreatLevel != EAudio_ThreatLevel::Safe)
    {
        float NormalizedProximity = FMath::Clamp(
            1.0f - (CurrentThreatState.ThreatProximity / 2000.0f),
            0.0f,
            1.0f
        );
        ProximityFactor = 1.0f + NormalizedProximity * 0.5f;
    }

    return FMath::Clamp(BaseIntensity * ThreatMultiplier * ProximityFactor, 0.0f, 1.0f);
}
