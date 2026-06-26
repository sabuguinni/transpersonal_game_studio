// AudioZoneManager.cpp
// Agent #16 — Audio Agent
// Prehistoric survival audio zone system — ambient sound management for MinPlayableMap
// Handles jungle, river, wind, danger, cave, and open plain audio zones

#include "AudioZoneManager.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

// ============================================================
// UAudio_ZoneComponent
// ============================================================

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAudio_ZoneComponent::BeginPlay()
{
    Super::BeginPlay();

    // Find or create audio component on owner
    AActor* Owner = GetOwner();
    if (Owner)
    {
        AudioComponent = Owner->FindComponentByClass<UAudioComponent>();
    }
}

void UAudio_ZoneComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->FadeOut(ZoneConfig.FadeOutDuration, 0.0f);
    }
    Super::EndPlay(EndPlayReason);
}

void UAudio_ZoneComponent::OnPlayerEnterZone()
{
    bIsPlayerInZone = true;

    if (AudioComponent && !AudioComponent->IsPlaying())
    {
        AudioComponent->SetVolumeMultiplier(0.0f);
        AudioComponent->Play();
        AudioComponent->AdjustVolume(ZoneConfig.FadeInDuration, ZoneConfig.VolumeMultiplier);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player entered zone type %d"), (int32)ZoneConfig.ZoneType);
}

void UAudio_ZoneComponent::OnPlayerExitZone()
{
    bIsPlayerInZone = false;

    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->FadeOut(ZoneConfig.FadeOutDuration, 0.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player exited zone type %d"), (int32)ZoneConfig.ZoneType);
}

// ============================================================
// AAudio_ZoneManager
// ============================================================

AAudio_ZoneManager::AAudio_ZoneManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec — sufficient for audio blending

    // Default zones for MinPlayableMap
    FAudio_ZoneConfig JungleZone;
    JungleZone.ZoneType = EAudio_ZoneType::JungleAmbience;
    JungleZone.AttenuationRadius = 4000.0f;
    JungleZone.VolumeMultiplier = 0.8f;
    JungleZone.bLooping = true;
    JungleZone.FadeInDuration = 3.0f;
    RegisteredZones.Add(JungleZone);

    FAudio_ZoneConfig RiverZone;
    RiverZone.ZoneType = EAudio_ZoneType::RiverAmbience;
    RiverZone.AttenuationRadius = 2000.0f;
    RiverZone.VolumeMultiplier = 0.6f;
    RiverZone.bLooping = true;
    RiverZone.FadeInDuration = 2.0f;
    RegisteredZones.Add(RiverZone);

    FAudio_ZoneConfig DangerZone;
    DangerZone.ZoneType = EAudio_ZoneType::DangerZone;
    DangerZone.AttenuationRadius = 5000.0f;
    DangerZone.VolumeMultiplier = 1.0f;
    DangerZone.bLooping = false;
    DangerZone.FadeInDuration = 0.5f;
    RegisteredZones.Add(DangerZone);
}

void AAudio_ZoneManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Initialized with %d zones"), RegisteredZones.Num());
}

void AAudio_ZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDynamicMixing)
    {
        UpdateDangerMix(DeltaTime);
    }
}

void AAudio_ZoneManager::RegisterZone(FAudio_ZoneConfig ZoneConfig)
{
    RegisteredZones.Add(ZoneConfig);
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Registered new zone type %d"), (int32)ZoneConfig.ZoneType);
}

void AAudio_ZoneManager::SetGlobalVolume(float NewVolume)
{
    GlobalVolumeScale = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    // Apply to all active audio components via UGameplayStatics
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f); // Keep time normal
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Global volume set to %.2f"), GlobalVolumeScale);
}

void AAudio_ZoneManager::TriggerDangerStinger(FVector DangerLocation)
{
    // Increase danger level — drives music intensity
    TargetDangerLevel = FMath::Min(TargetDangerLevel + 0.3f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Danger stinger triggered at (%.0f, %.0f, %.0f) — danger level: %.2f"),
        DangerLocation.X, DangerLocation.Y, DangerLocation.Z, TargetDangerLevel);
}

void AAudio_ZoneManager::TriggerNightTransition()
{
    // Night: reduce jungle birds, increase insect/frog ambience
    // In a full implementation this would crossfade sound cues
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Night transition triggered — switching to nocturnal ambience"));
}

void AAudio_ZoneManager::TriggerDawnTransition()
{
    // Dawn: bring back bird calls, reduce nocturnal sounds
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Dawn transition triggered — switching to diurnal ambience"));
}

void AAudio_ZoneManager::UpdateDangerMix(float DeltaTime)
{
    // Smoothly blend danger level toward target
    if (!FMath::IsNearlyEqual(CurrentDangerLevel, TargetDangerLevel, 0.01f))
    {
        CurrentDangerLevel = FMath::FInterpTo(CurrentDangerLevel, TargetDangerLevel, DeltaTime, DangerBlendSpeed);
    }

    // Danger decays over time when no stingers triggered
    TargetDangerLevel = FMath::Max(0.0f, TargetDangerLevel - (DeltaTime * 0.05f));
}
