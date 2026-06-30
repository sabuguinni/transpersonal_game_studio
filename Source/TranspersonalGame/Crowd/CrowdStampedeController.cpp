// CrowdStampedeController.cpp
// Agent #13 — Crowd & Traffic Simulation
// Stampede system: triggers, wave propagation, panic spread, danger zones

#include "CrowdStampedeController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ACrowd_StampedeController::ACrowd_StampedeController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick for performance

    MaxSimultaneousStampedes = 3;
    GlobalPanicDecayRate = 0.05f;
    StampedeWaveSpeed = 800.0f;
    bStampedeActive = false;
    GlobalPanicLevel = 0.0f;
}

void ACrowd_StampedeController::BeginPlay()
{
    Super::BeginPlay();
    ActiveWaves.Empty();
    ActiveDangerZones.Empty();
    GlobalPanicLevel = 0.0f;
    bStampedeActive = false;
    UE_LOG(LogTemp, Log, TEXT("[CrowdStampede] StampedeController initialized. MaxSimultaneous=%d"), MaxSimultaneousStampedes);
}

void ACrowd_StampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateStampedeWaves(DeltaTime);
    UpdateDangerZones(DeltaTime);
    DecayGlobalPanic(DeltaTime);
}

void ACrowd_StampedeController::TriggerStampede(ECrowd_StampedeTrigger TriggerType, FVector Origin, float Intensity)
{
    if (ActiveWaves.Num() >= MaxSimultaneousStampedes)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdStampede] Max simultaneous stampedes reached (%d). Ignoring new trigger."), MaxSimultaneousStampedes);
        return;
    }

    FCrowd_StampedeWave NewWave;
    NewWave.Origin = Origin;
    NewWave.PropagationRadius = 0.0f;
    NewWave.PropagationSpeed = StampedeWaveSpeed;
    NewWave.PanicIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    NewWave.ElapsedTime = 0.0f;
    NewWave.bIsActive = true;
    NewWave.TriggerType = TriggerType;

    // Scale max radius by trigger type
    switch (TriggerType)
    {
        case ECrowd_StampedeTrigger::Explosion:
            NewWave.MaxPropagationRadius = 5000.0f;
            NewWave.PanicIntensity = FMath::Max(NewWave.PanicIntensity, 0.9f);
            break;
        case ECrowd_StampedeTrigger::PredatorDetected:
            NewWave.MaxPropagationRadius = 3500.0f;
            break;
        case ECrowd_StampedeTrigger::LightningStrike:
            NewWave.MaxPropagationRadius = 4000.0f;
            NewWave.PanicIntensity = FMath::Max(NewWave.PanicIntensity, 0.75f);
            break;
        case ECrowd_StampedeTrigger::LoudNoise:
            NewWave.MaxPropagationRadius = 2500.0f;
            break;
        case ECrowd_StampedeTrigger::AlphaFleeSignal:
            NewWave.MaxPropagationRadius = 4500.0f;
            NewWave.PanicIntensity = FMath::Max(NewWave.PanicIntensity, 0.85f);
            break;
        case ECrowd_StampedeTrigger::PlayerProximity:
        default:
            NewWave.MaxPropagationRadius = 2000.0f;
            break;
    }

    ActiveWaves.Add(NewWave);
    bStampedeActive = true;

    // Boost global panic
    GlobalPanicLevel = FMath::Min(GlobalPanicLevel + NewWave.PanicIntensity * 0.4f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("[CrowdStampede] Stampede triggered! Type=%d Origin=(%.0f,%.0f,%.0f) Intensity=%.2f MaxRadius=%.0f"),
        (int32)TriggerType, Origin.X, Origin.Y, Origin.Z, NewWave.PanicIntensity, NewWave.MaxPropagationRadius);

    OnStampedeTriggered.Broadcast(TriggerType, Origin, NewWave.PanicIntensity);
}

void ACrowd_StampedeController::AddDangerZone(FVector Center, float Radius, float DangerLevel, float Duration)
{
    FCrowd_DangerZone Zone;
    Zone.Center = Center;
    Zone.Radius = FMath::Max(Radius, 100.0f);
    Zone.DangerLevel = FMath::Clamp(DangerLevel, 0.0f, 1.0f);
    Zone.RemainingDuration = Duration;
    Zone.bIsPersistent = (Duration <= 0.0f);

    ActiveDangerZones.Add(Zone);

    UE_LOG(LogTemp, Log, TEXT("[CrowdStampede] DangerZone added at (%.0f,%.0f,%.0f) Radius=%.0f Danger=%.2f Duration=%.1fs"),
        Center.X, Center.Y, Center.Z, Radius, DangerLevel, Duration);
}

float ACrowd_StampedeController::GetPanicLevelAtLocation(FVector Location) const
{
    float MaxPanic = GlobalPanicLevel * 0.3f; // Base from global panic

    // Check active stampede waves
    for (const FCrowd_StampedeWave& Wave : ActiveWaves)
    {
        if (!Wave.bIsActive) continue;
        float Dist = FVector::Dist(Location, Wave.Origin);
        if (Dist <= Wave.PropagationRadius)
        {
            // Panic falls off from origin outward within the wave front
            float WaveFrontDist = FMath::Abs(Dist - Wave.PropagationRadius);
            float WaveFalloff = FMath::Clamp(1.0f - WaveFrontDist / 500.0f, 0.0f, 1.0f);
            float LocalPanic = Wave.PanicIntensity * WaveFalloff;
            MaxPanic = FMath::Max(MaxPanic, LocalPanic);
        }
    }

    // Check danger zones
    for (const FCrowd_DangerZone& Zone : ActiveDangerZones)
    {
        float Dist = FVector::Dist(Location, Zone.Center);
        if (Dist <= Zone.Radius)
        {
            float Falloff = FMath::Clamp(1.0f - (Dist / Zone.Radius), 0.0f, 1.0f);
            float ZonePanic = Zone.DangerLevel * Falloff;
            MaxPanic = FMath::Max(MaxPanic, ZonePanic);
        }
    }

    return FMath::Clamp(MaxPanic, 0.0f, 1.0f);
}

void ACrowd_StampedeController::UpdateStampedeWaves(float DeltaTime)
{
    bool bAnyActive = false;

    for (FCrowd_StampedeWave& Wave : ActiveWaves)
    {
        if (!Wave.bIsActive) continue;

        Wave.ElapsedTime += DeltaTime;
        Wave.PropagationRadius += Wave.PropagationSpeed * DeltaTime;

        // Decay panic intensity as wave expands
        Wave.PanicIntensity = FMath::Max(Wave.PanicIntensity - 0.02f * DeltaTime, 0.0f);

        if (Wave.PropagationRadius >= Wave.MaxPropagationRadius || Wave.PanicIntensity <= 0.01f)
        {
            Wave.bIsActive = false;
            UE_LOG(LogTemp, Log, TEXT("[CrowdStampede] Wave expired after %.1fs, radius=%.0f"), Wave.ElapsedTime, Wave.PropagationRadius);
        }
        else
        {
            bAnyActive = true;
        }
    }

    // Remove expired waves
    ActiveWaves.RemoveAll([](const FCrowd_StampedeWave& W) { return !W.bIsActive; });

    if (bStampedeActive && !bAnyActive && ActiveWaves.Num() == 0)
    {
        bStampedeActive = false;
        OnStampedeEnded.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("[CrowdStampede] All stampede waves expired. Herd calming."));
    }
}

void ACrowd_StampedeController::UpdateDangerZones(float DeltaTime)
{
    for (FCrowd_DangerZone& Zone : ActiveDangerZones)
    {
        if (Zone.bIsPersistent) continue;
        Zone.RemainingDuration -= DeltaTime;
    }

    // Remove expired non-persistent zones
    ActiveDangerZones.RemoveAll([](const FCrowd_DangerZone& Z)
    {
        return !Z.bIsPersistent && Z.RemainingDuration <= 0.0f;
    });
}

void ACrowd_StampedeController::DecayGlobalPanic(float DeltaTime)
{
    if (GlobalPanicLevel > 0.0f)
    {
        GlobalPanicLevel = FMath::Max(GlobalPanicLevel - GlobalPanicDecayRate * DeltaTime, 0.0f);
    }
}

void ACrowd_StampedeController::ClearAllDangerZones()
{
    int32 Removed = ActiveDangerZones.Num();
    ActiveDangerZones.Empty();
    UE_LOG(LogTemp, Log, TEXT("[CrowdStampede] Cleared %d danger zones."), Removed);
}
