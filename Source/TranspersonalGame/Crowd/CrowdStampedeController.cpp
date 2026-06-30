#include "CrowdStampedeController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ACrowd_StampedeController::ACrowd_StampedeController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 Hz — sufficient for wave propagation
}

void ACrowd_StampedeController::BeginPlay()
{
    Super::BeginPlay();
    bStampedeActive = false;
    StampedeElapsedTime = 0.0f;
    ActiveWaves.Empty();
    ActiveDangerZones.Empty();
}

void ACrowd_StampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bStampedeActive)
    {
        return;
    }

    StampedeElapsedTime += DeltaTime;
    UpdateWaves(DeltaTime);
    UpdateDangerZones(DeltaTime);

    // Calm-down logic
    CalmDownTimer += DeltaTime;
    if (CalmDownTimer >= CalmDownTime)
    {
        StopStampede();
    }
}

// ── Public API ───────────────────────────────────────────────────────────────

void ACrowd_StampedeController::TriggerStampede(
    ECrowd_StampedeTrigger Trigger,
    FVector TriggerLocation,
    FVector FleeDirection)
{
    if (bStampedeActive)
    {
        // Reinforce existing stampede — add a new wave
    }

    bStampedeActive = true;
    StampedeElapsedTime = 0.0f;
    CalmDownTimer = 0.0f;
    StampedeDirection = FleeDirection.GetSafeNormal();

    // Spawn initial panic wave
    FCrowd_StampedeWave InitialWave;
    InitialWave.Origin = TriggerLocation;
    InitialWave.PropagationRadius = 0.0f;
    InitialWave.PropagationSpeed = 1200.0f;
    InitialWave.PanicIntensity = 1.0f;
    InitialWave.ElapsedTime = 0.0f;
    InitialWave.bActive = true;
    ActiveWaves.Add(InitialWave);

    // Spawn initial danger zone at trigger point
    float DPS = 20.0f;
    float ZoneRadius = 400.0f;
    if (Trigger == ECrowd_StampedeTrigger::Explosion)
    {
        DPS = 50.0f;
        ZoneRadius = 800.0f;
    }
    SpawnDangerZoneAtLocation(TriggerLocation, ZoneRadius, DPS, 6.0f);

    // Notify nearby herds
    NotifyNearbyHerds(TriggerLocation, 1.0f);

    UE_LOG(LogTemp, Warning, TEXT("ACrowd_StampedeController: Stampede triggered at %s, direction %s"),
        *TriggerLocation.ToString(), *StampedeDirection.ToString());
}

void ACrowd_StampedeController::StopStampede()
{
    bStampedeActive = false;
    StampedeElapsedTime = 0.0f;
    CalmDownTimer = 0.0f;
    ActiveWaves.Empty();
    // Danger zones persist until their own timers expire
    UE_LOG(LogTemp, Log, TEXT("ACrowd_StampedeController: Stampede calmed down."));
}

bool ACrowd_StampedeController::IsLocationInDangerZone(FVector Location) const
{
    for (const FCrowd_DangerZone& Zone : ActiveDangerZones)
    {
        if (FVector::DistSquared(Location, Zone.Center) <= Zone.Radius * Zone.Radius)
        {
            return true;
        }
    }
    return false;
}

float ACrowd_StampedeController::GetPanicIntensityAtLocation(FVector Location) const
{
    float MaxIntensity = 0.0f;
    for (const FCrowd_StampedeWave& Wave : ActiveWaves)
    {
        if (!Wave.bActive) continue;
        float Dist = FVector::Dist(Location, Wave.Origin);
        if (Dist <= Wave.PropagationRadius)
        {
            // Intensity decays linearly from origin to wave front
            float NormalizedDist = (Wave.PropagationRadius > 0.0f)
                ? (Dist / Wave.PropagationRadius)
                : 0.0f;
            float LocalIntensity = Wave.PanicIntensity * (1.0f - NormalizedDist * PanicDecayRate);
            MaxIntensity = FMath::Max(MaxIntensity, FMath::Clamp(LocalIntensity, 0.0f, 1.0f));
        }
    }
    return MaxIntensity;
}

void ACrowd_StampedeController::DEBUG_TriggerTestStampede()
{
    FVector Loc = GetActorLocation();
    FVector Dir = GetActorForwardVector();
    TriggerStampede(ECrowd_StampedeTrigger::PlayerProximity, Loc, Dir);
    UE_LOG(LogTemp, Warning, TEXT("DEBUG: Test stampede triggered from editor."));
}

// ── Private helpers ──────────────────────────────────────────────────────────

void ACrowd_StampedeController::UpdateWaves(float DeltaTime)
{
    for (int32 i = ActiveWaves.Num() - 1; i >= 0; --i)
    {
        FCrowd_StampedeWave& Wave = ActiveWaves[i];
        if (!Wave.bActive) continue;

        Wave.ElapsedTime += DeltaTime;
        Wave.PropagationRadius += Wave.PropagationSpeed * DeltaTime;
        Wave.PanicIntensity = FMath::Max(0.0f, Wave.PanicIntensity - PanicDecayRate * DeltaTime);

        // Spawn secondary danger zones as wave expands
        if (FMath::Fmod(Wave.ElapsedTime, 1.5f) < DeltaTime)
        {
            // Every ~1.5s, create a danger zone at the wave front in the stampede direction
            FVector FrontPos = Wave.Origin + StampedeDirection * Wave.PropagationRadius;
            SpawnDangerZoneAtLocation(FrontPos, 300.0f, 15.0f * Wave.PanicIntensity, 4.0f);
        }

        if (Wave.PropagationRadius >= MaxWaveRadius || Wave.PanicIntensity <= 0.0f)
        {
            Wave.bActive = false;
            ActiveWaves.RemoveAt(i);
        }
    }
}

void ACrowd_StampedeController::UpdateDangerZones(float DeltaTime)
{
    for (int32 i = ActiveDangerZones.Num() - 1; i >= 0; --i)
    {
        FCrowd_DangerZone& Zone = ActiveDangerZones[i];
        Zone.ElapsedTime += DeltaTime;

        if (Zone.ElapsedTime >= Zone.Duration)
        {
            ActiveDangerZones.RemoveAt(i);
        }
    }
}

void ACrowd_StampedeController::SpawnDangerZoneAtLocation(
    FVector Location, float Radius, float DPS, float Duration)
{
    FCrowd_DangerZone NewZone;
    NewZone.Center = Location;
    NewZone.Radius = Radius;
    NewZone.DamagePerSecond = DPS;
    NewZone.Duration = Duration;
    NewZone.ElapsedTime = 0.0f;
    ActiveDangerZones.Add(NewZone);
}

void ACrowd_StampedeController::NotifyNearbyHerds(FVector Origin, float PanicIntensity)
{
    // Broadcast a gameplay event that herd behavior actors can listen to
    // This is a lightweight notification — actual herd response is handled by CrowdHerdBehavior
    UE_LOG(LogTemp, Log, TEXT("ACrowd_StampedeController: Notifying herds near %s with panic %.2f"),
        *Origin.ToString(), PanicIntensity);
}
