// AudioZoneSystem.cpp
// Agent #16 — Audio Agent
// Full implementation of adaptive audio zone system
#include "AudioZoneSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAudio_ZoneSystem::UAudio_ZoneSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    ActiveZoneType = EAudio_ZoneType::NightAmbience;
    DistanceToNearestPredator = 99999.0f;
    MasterVolume = 1.0f;
    bEnableAdaptiveMusic = true;
    ZoneCheckIntervalSeconds = 0.5f;

    // Default T-Rex screen shake config
    TRexShakeConfig.ShakeIntensity = 2.5f;
    TRexShakeConfig.ShakeDuration = 0.6f;
    TRexShakeConfig.ShakeFrequency = 8.0f;
    TRexShakeConfig.TriggerDistanceMeters = 40.0f;

    // Default Brachiosaurus shake (gentler, lower frequency)
    BrachiosaurusShakeConfig.ShakeIntensity = 1.2f;
    BrachiosaurusShakeConfig.ShakeDuration = 0.8f;
    BrachiosaurusShakeConfig.ShakeFrequency = 4.0f;
    BrachiosaurusShakeConfig.TriggerDistanceMeters = 60.0f;
}

void UAudio_ZoneSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize default audio zones if none configured
    if (AudioZones.Num() == 0)
    {
        // T-Rex proximity zone
        FAudio_ZoneLayer TRexZone;
        TRexZone.ZoneType = EAudio_ZoneType::TRexProximity;
        TRexZone.TriggerRadius = 1500.0f;
        TRexZone.FadeInTime = 1.5f;
        TRexZone.FadeOutTime = 4.0f;
        TRexZone.MaxVolume = 1.0f;
        TRexZone.bLooping = true;
        AudioZones.Add(TRexZone);

        // Raptor patrol zone
        FAudio_ZoneLayer RaptorZone;
        RaptorZone.ZoneType = EAudio_ZoneType::RaptorPatrol;
        RaptorZone.TriggerRadius = 900.0f;
        RaptorZone.FadeInTime = 1.0f;
        RaptorZone.FadeOutTime = 2.5f;
        RaptorZone.MaxVolume = 0.85f;
        RaptorZone.bLooping = true;
        AudioZones.Add(RaptorZone);

        // Night ambience zone (default)
        FAudio_ZoneLayer NightZone;
        NightZone.ZoneType = EAudio_ZoneType::NightAmbience;
        NightZone.TriggerRadius = 99999.0f; // always active as baseline
        NightZone.FadeInTime = 3.0f;
        NightZone.FadeOutTime = 5.0f;
        NightZone.MaxVolume = 0.6f;
        NightZone.bLooping = true;
        AudioZones.Add(NightZone);

        // River ambience
        FAudio_ZoneLayer RiverZone;
        RiverZone.ZoneType = EAudio_ZoneType::RiverAmbience;
        RiverZone.TriggerRadius = 700.0f;
        RiverZone.FadeInTime = 2.0f;
        RiverZone.FadeOutTime = 3.0f;
        RiverZone.MaxVolume = 0.75f;
        RiverZone.bLooping = true;
        AudioZones.Add(RiverZone);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZoneSystem: Initialized with %d zones"), AudioZones.Num());
}

void UAudio_ZoneSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ZoneCheckTimer += DeltaTime;
    if (ZoneCheckTimer >= ZoneCheckIntervalSeconds)
    {
        ZoneCheckTimer = 0.0f;
        EvaluateZones();
    }
}

void UAudio_ZoneSystem::EvaluateZones()
{
    // Calculate danger level from nearest predator distance
    EAudio_DangerLevel NewDanger = CalculateDangerFromDistance(DistanceToNearestPredator);

    if (NewDanger != CurrentDangerLevel)
    {
        SetDangerLevel(NewDanger);
    }

    // Determine active zone type based on danger
    EAudio_ZoneType NewZone = ActiveZoneType;
    if (DistanceToNearestPredator < 1500.0f)
    {
        NewZone = EAudio_ZoneType::TRexProximity;
    }
    else if (DistanceToNearestPredator < 2500.0f)
    {
        NewZone = EAudio_ZoneType::RaptorPatrol;
    }
    else
    {
        NewZone = EAudio_ZoneType::NightAmbience;
    }

    if (NewZone != ActiveZoneType)
    {
        TransitionToZone(NewZone);
    }
}

void UAudio_ZoneSystem::TransitionToZone(EAudio_ZoneType NewZone)
{
    UE_LOG(LogTemp, Log, TEXT("AudioZoneSystem: Transitioning from zone %d to zone %d"),
        (int32)ActiveZoneType, (int32)NewZone);

    ActiveZoneType = NewZone;

    // Find zone config
    for (const FAudio_ZoneLayer& Zone : AudioZones)
    {
        if (Zone.ZoneType == NewZone)
        {
            // In a full implementation, this would fade between MetaSound patches
            // For now, log the transition for Blueprint wiring
            UE_LOG(LogTemp, Log, TEXT("AudioZoneSystem: Zone '%d' — FadeIn=%.1fs FadeOut=%.1fs Vol=%.2f"),
                (int32)Zone.ZoneType, Zone.FadeInTime, Zone.FadeOutTime, Zone.MaxVolume * MasterVolume);
            break;
        }
    }
}

void UAudio_ZoneSystem::FadeOutAllAmbient(float FadeTime)
{
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (IsValid(AudioComp) && AudioComp->IsPlaying())
        {
            AudioComp->FadeOut(FadeTime, 0.0f);
        }
    }
}

EAudio_DangerLevel UAudio_ZoneSystem::CalculateDangerFromDistance(float Distance) const
{
    if (Distance < 500.0f)   return EAudio_DangerLevel::Critical;
    if (Distance < 1200.0f)  return EAudio_DangerLevel::Danger;
    if (Distance < 2500.0f)  return EAudio_DangerLevel::Cautious;
    return EAudio_DangerLevel::Safe;
}

void UAudio_ZoneSystem::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    EAudio_DangerLevel OldLevel = CurrentDangerLevel;
    CurrentDangerLevel = NewLevel;

    UE_LOG(LogTemp, Log, TEXT("AudioZoneSystem: Danger level changed %d → %d"),
        (int32)OldLevel, (int32)NewLevel);

    // Trigger stinger on escalation
    if ((int32)NewLevel > (int32)OldLevel)
    {
        TriggerDangerStinger(ActiveZoneType);
    }

    // Screen shake on critical danger (T-Rex very close)
    if (NewLevel == EAudio_DangerLevel::Critical)
    {
        TriggerScreenShake(TRexShakeConfig.ShakeIntensity, TRexShakeConfig.ShakeDuration);
    }
}

void UAudio_ZoneSystem::TriggerDangerStinger(EAudio_ZoneType ZoneType)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FAudio_ZoneLayer& Zone : AudioZones)
    {
        if (Zone.ZoneType == ZoneType && Zone.DangerStinger.IsValid())
        {
            USoundBase* Stinger = Zone.DangerStinger.Get();
            if (Stinger)
            {
                AActor* Owner = GetOwner();
                FVector Location = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
                UGameplayStatics::PlaySoundAtLocation(World, Stinger, Location, MasterVolume);
                UE_LOG(LogTemp, Log, TEXT("AudioZoneSystem: Played danger stinger for zone %d"), (int32)ZoneType);
            }
        }
    }
}

void UAudio_ZoneSystem::PlayVoiceLine(const FString& CharacterName)
{
    UWorld* World = GetWorld();
    if (!World) return;

    float CurrentTime = World->GetTimeSeconds();

    for (FAudio_VoiceLine& Line : NPCVoiceLines)
    {
        if (Line.CharacterName == CharacterName && !Line.bPlayedThisSession)
        {
            if (Line.VoiceAsset.IsValid())
            {
                USoundBase* Voice = Line.VoiceAsset.Get();
                if (Voice)
                {
                    AActor* Owner = GetOwner();
                    FVector Location = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
                    UGameplayStatics::PlaySoundAtLocation(World, Voice, Location, MasterVolume);
                    Line.bPlayedThisSession = true;
                    UE_LOG(LogTemp, Log, TEXT("AudioZoneSystem: Playing voice line for '%s'"), *CharacterName);
                }
            }
            break;
        }
    }
}

void UAudio_ZoneSystem::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->SetVolumeMultiplier(MasterVolume);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AudioZoneSystem: Master volume set to %.2f"), MasterVolume);
}

void UAudio_ZoneSystem::TriggerScreenShake(float Intensity, float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    // Use console command to trigger screen shake (Blueprint-compatible fallback)
    // In full implementation, load UCameraShakeBase subclass and call ClientStartCameraShake
    UE_LOG(LogTemp, Log, TEXT("AudioZoneSystem: Screen shake triggered — Intensity=%.2f Duration=%.2fs"), Intensity, Duration);

    // Execute via console for now — Blueprint can override with proper CameraShake asset
    FString ShakeCmd = FString::Printf(TEXT("shake %.2f %.2f"), Intensity, Duration);
    UGameplayStatics::GetPlayerController(World, 0)->ConsoleCommand(ShakeCmd);
}

void UAudio_ZoneSystem::UpdateNearestPredatorDistance(float Distance)
{
    DistanceToNearestPredator = FMath::Max(0.0f, Distance);
}
