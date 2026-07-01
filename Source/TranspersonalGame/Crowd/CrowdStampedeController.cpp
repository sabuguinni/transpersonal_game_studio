// CrowdStampedeController.cpp
// Crowd & Traffic Simulation Agent #13 — Cycle AUTO_20260701_004
// Stampede Controller — coordinates mass panic events across multiple herds

#include "CrowdStampedeController.h"
#include "CrowdHerdMigration.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─── ACrowdStampedeController ───────────────────────────────────────────────

ACrowdStampedeController::ACrowdStampedeController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz for stampede responsiveness

    bGlobalStampedeActive = false;
    PanicPropagationRadius = 2500.0f;
    PanicPropagationSpeed = 800.0f;
    MaxConcurrentStampedes = 3;
    StampedeDecayRate = 0.1f;
    CurrentStampedeIntensity = 0.0f;
}

void ACrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();

    // Discover all herd actors in the world
    DiscoverHerds();

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Initialized — %d herds registered"),
        RegisteredHerds.Num());
}

void ACrowdStampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bGlobalStampedeActive) return;

    // Propagate panic wave outward
    PropagateStampedePanic(DeltaTime);

    // Decay stampede intensity
    CurrentStampedeIntensity = FMath::Max(0.0f,
        CurrentStampedeIntensity - StampedeDecayRate * DeltaTime);

    if (CurrentStampedeIntensity <= 0.0f)
    {
        EndGlobalStampede();
    }
}

void ACrowdStampedeController::DiscoverHerds()
{
    RegisteredHerds.Empty();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(),
        ACrowdHerdMigration::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        ACrowdHerdMigration* Herd = Cast<ACrowdHerdMigration>(Actor);
        if (Herd)
        {
            RegisteredHerds.Add(Herd);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Discovered %d herds"),
        RegisteredHerds.Num());
}

void ACrowdStampedeController::RegisterHerd(ACrowdHerdMigration* Herd)
{
    if (Herd && !RegisteredHerds.Contains(Herd))
    {
        RegisteredHerds.Add(Herd);
        UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Registered herd %s"),
            *Herd->GetName());
    }
}

void ACrowdStampedeController::TriggerGlobalStampede(
    FVector EpicentreLocation,
    ECrowd_StampedeType StampedeType,
    float Intensity)
{
    if (bGlobalStampedeActive && ActiveStampedeEvents.Num() >= MaxConcurrentStampedes)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdStampedeController: Max concurrent stampedes reached"));
        return;
    }

    // Create stampede event
    FCrowd_StampedeEvent NewEvent;
    NewEvent.TriggerLocation = EpicentreLocation;
    NewEvent.StampedeType = StampedeType;
    NewEvent.TriggerRadius = PanicPropagationRadius;
    NewEvent.AgentSpeed = PanicPropagationSpeed;
    NewEvent.Duration = 15.0f + (Intensity * 10.0f);
    NewEvent.AffectedAgentCount = 0;
    NewEvent.bIsActive = true;

    ActiveStampedeEvents.Add(NewEvent);

    bGlobalStampedeActive = true;
    CurrentStampedeIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    GlobalEpicentre = EpicentreLocation;

    // Trigger all nearby herds
    for (ACrowdHerdMigration* Herd : RegisteredHerds)
    {
        if (!Herd) continue;

        float DistToHerd = FVector::Dist(Herd->GetActorLocation(), EpicentreLocation);
        if (DistToHerd < PanicPropagationRadius)
        {
            float LocalRadius = PanicPropagationRadius - DistToHerd;
            Herd->TriggerStampede(EpicentreLocation, LocalRadius);
            NewEvent.AffectedAgentCount += Herd->GetActiveAgentCount();
        }
    }

    UE_LOG(LogTemp, Warning,
        TEXT("CrowdStampedeController: GLOBAL STAMPEDE — type %d, epicentre %s, intensity %.2f, %d agents affected"),
        (int32)StampedeType, *EpicentreLocation.ToString(), Intensity, NewEvent.AffectedAgentCount);

    OnGlobalStampedeStarted.Broadcast(EpicentreLocation, (int32)StampedeType);

    // Schedule auto-end
    FTimerHandle EndTimer;
    GetWorldTimerManager().SetTimer(EndTimer, this,
        &ACrowdStampedeController::EndGlobalStampede, NewEvent.Duration, false);
}

void ACrowdStampedeController::TriggerDirectionalRush(
    FVector StartLocation,
    FVector RushDirection,
    float Width,
    float Speed)
{
    // Directional rush — all agents in a corridor move in one direction
    FVector NormDir = RushDirection.GetSafeNormal();

    for (ACrowdHerdMigration* Herd : RegisteredHerds)
    {
        if (!Herd) continue;

        FVector ToHerd = Herd->GetActorLocation() - StartLocation;
        float LateralDist = FVector::CrossProduct(ToHerd, NormDir).Size();

        if (LateralDist < Width)
        {
            // Herd is in the rush corridor
            Herd->TriggerStampede(StartLocation - NormDir * 500.0f, Width);
            UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Directional rush hit herd %s"),
                *Herd->GetName());
        }
    }

    UE_LOG(LogTemp, Log,
        TEXT("CrowdStampedeController: Directional rush — dir %s, width %.0f, speed %.0f"),
        *NormDir.ToString(), Width, Speed);
}

void ACrowdStampedeController::TriggerScatterPattern(FVector CentreLocation, float ScatterRadius)
{
    // Scatter — agents flee outward in all directions from centre
    for (ACrowdHerdMigration* Herd : RegisteredHerds)
    {
        if (!Herd) continue;

        float DistToHerd = FVector::Dist(Herd->GetActorLocation(), CentreLocation);
        if (DistToHerd < ScatterRadius)
        {
            Herd->TriggerStampede(CentreLocation, ScatterRadius);
        }
    }

    UE_LOG(LogTemp, Log,
        TEXT("CrowdStampedeController: Scatter pattern — centre %s, radius %.0f"),
        *CentreLocation.ToString(), ScatterRadius);
}

void ACrowdStampedeController::PropagateStampedePanic(float DeltaTime)
{
    // Expand panic wave radius over time
    for (FCrowd_StampedeEvent& Event : ActiveStampedeEvents)
    {
        if (!Event.bIsActive) continue;

        Event.TriggerRadius += PanicPropagationSpeed * DeltaTime;

        // Check for newly affected herds as wave expands
        for (ACrowdHerdMigration* Herd : RegisteredHerds)
        {
            if (!Herd) continue;

            float DistToHerd = FVector::Dist(Herd->GetActorLocation(), Event.TriggerLocation);
            // Trigger herds at the wave front (within 200 units of expanding radius)
            if (FMath::Abs(DistToHerd - Event.TriggerRadius) < 200.0f)
            {
                if (!Herd->IsStampedeActive())
                {
                    Herd->TriggerStampede(Event.TriggerLocation, 800.0f);
                    Event.AffectedAgentCount += Herd->GetActiveAgentCount();
                }
            }
        }
    }
}

void ACrowdStampedeController::EndGlobalStampede()
{
    bGlobalStampedeActive = false;
    CurrentStampedeIntensity = 0.0f;

    for (FCrowd_StampedeEvent& Event : ActiveStampedeEvents)
    {
        Event.bIsActive = false;
    }
    ActiveStampedeEvents.Empty();

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Global stampede ended — herds calming"));
    OnGlobalStampedeEnded.Broadcast();
}

int32 ACrowdStampedeController::GetTotalAffectedAgents() const
{
    int32 Total = 0;
    for (const FCrowd_StampedeEvent& Event : ActiveStampedeEvents)
    {
        Total += Event.AffectedAgentCount;
    }
    return Total;
}

float ACrowdStampedeController::GetStampedeIntensity() const
{
    return CurrentStampedeIntensity;
}

bool ACrowdStampedeController::IsStampedeActive() const
{
    return bGlobalStampedeActive;
}
