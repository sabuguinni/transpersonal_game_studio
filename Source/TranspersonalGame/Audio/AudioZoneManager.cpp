#include "AudioZoneManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UAudioZoneManager::UAudioZoneManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentVolume = 0.0f;
    TargetVolume = 0.0f;
    bPlayerInZone = false;
}

void UAudioZoneManager::BeginPlay()
{
    Super::BeginPlay();
    CurrentVolume = 0.0f;
    TargetVolume = 0.0f;
}

void UAudioZoneManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Smoothly blend volume toward target
    if (!FMath::IsNearlyEqual(CurrentVolume, TargetVolume, 0.001f))
    {
        CurrentVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, TransitionSpeed);
    }
}

void UAudioZoneManager::EnterZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;
    bPlayerInZone = true;
    TargetVolume = ZoneConfig.AmbientVolume;

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Player entered zone type %d — danger=%.2f"),
        (int32)ZoneConfig.ZoneType, ZoneConfig.DangerLevel);
}

void UAudioZoneManager::ExitZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;
    bPlayerInZone = false;
    TargetVolume = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Player exited zone type %d"),
        (int32)ZoneConfig.ZoneType);
}

float UAudioZoneManager::GetCurrentDangerLevel() const
{
    return bPlayerInZone ? ZoneConfig.DangerLevel : 0.0f;
}

EAudio_ZoneType UAudioZoneManager::GetZoneType() const
{
    return ZoneConfig.ZoneType;
}

void UAudioZoneManager::SetMusicIntensity(float NewIntensity)
{
    ZoneConfig.MusicIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioZoneManager: Music intensity set to %.2f"), ZoneConfig.MusicIntensity);
}
