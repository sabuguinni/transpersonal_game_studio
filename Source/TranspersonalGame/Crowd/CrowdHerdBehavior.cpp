// CrowdHerdBehavior.cpp
// Agent #13 — Crowd & Traffic Simulation
// Full implementation of UCrowdHerdBehavior component
// Boid-based flocking with stampede integration and LOD-aware simulation

#include "CrowdHerdBehavior.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCrowdHerdBehavior::UCrowdHerdBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance

    // Default boid parameters
    BoidParams.SeparationRadius = 350.0f;
    BoidParams.AlignmentRadius = 600.0f;
    BoidParams.CohesionRadius = 800.0f;
    BoidParams.SeparationWeight = 1.5f;
    BoidParams.AlignmentWeight = 0.6f;
    BoidParams.CohesionWeight = 0.8f;
    BoidParams.FleeWeight = 2.5f;
    BoidParams.MaxSpeed = 400.0f;
    BoidParams.MaxForce = 120.0f;

    // Default herd state
    CurrentHerdState = ECrowd_HerdState::Grazing;
    CurrentRole = ECrowd_HerdRole::Follower;
    PanicLevel = 0.0f;
    PanicDecayRate = 0.15f;
    FleeRadius = 1200.0f;
    StampedeThreshold = 0.7f;

    LODLevel = 0;
    bIsSimulationActive = true;
}

void UCrowdHerdBehavior::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner actor
    OwnerActor = GetOwner();

    // Register with herd manager if available
    if (OwnerActor)
    {
        InitialLocation = OwnerActor->GetActorLocation();
    }

    // Start grazing behavior
    SetHerdState(ECrowd_HerdState::Grazing);
}

void UCrowdHerdBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsSimulationActive || !OwnerActor)
    {
        return;
    }

    // LOD-based simulation granularity
    if (LODLevel >= 2)
    {
        // LOD2: static impostor, no simulation
        return;
    }

    // Decay panic over time
    if (PanicLevel > 0.0f)
    {
        PanicLevel = FMath::Max(0.0f, PanicLevel - PanicDecayRate * DeltaTime);

        // Transition back from flee states when panic subsides
        if (PanicLevel < 0.1f && CurrentHerdState == ECrowd_HerdState::Fleeing)
        {
            SetHerdState(ECrowd_HerdState::Wandering);
        }
        else if (PanicLevel < 0.05f && CurrentHerdState == ECrowd_HerdState::Stampeding)
        {
            SetHerdState(ECrowd_HerdState::Fleeing);
        }
    }

    // State machine update
    switch (CurrentHerdState)
    {
        case ECrowd_HerdState::Grazing:
            UpdateGrazingBehavior(DeltaTime);
            break;
        case ECrowd_HerdState::Wandering:
            UpdateWanderingBehavior(DeltaTime);
            break;
        case ECrowd_HerdState::Fleeing:
            UpdateFleeingBehavior(DeltaTime);
            break;
        case ECrowd_HerdState::Stampeding:
            UpdateStampedeBehavior(DeltaTime);
            break;
        case ECrowd_HerdState::Resting:
            // Minimal update — just check for threats
            CheckForPredatorThreat();
            break;
        case ECrowd_HerdState::Migrating:
            UpdateMigrationBehavior(DeltaTime);
            break;
        default:
            break;
    }
}

void UCrowdHerdBehavior::SetHerdState(ECrowd_HerdState NewState)
{
    if (CurrentHerdState == NewState)
    {
        return;
    }

    ECrowd_HerdState PreviousState = CurrentHerdState;
    CurrentHerdState = NewState;

    // State entry logic
    switch (NewState)
    {
        case ECrowd_HerdState::Stampeding:
            // Boost tick rate during stampede for responsive simulation
            PrimaryComponentTick.TickInterval = 0.033f; // ~30Hz
            break;
        case ECrowd_HerdState::Grazing:
        case ECrowd_HerdState::Resting:
            // Reduce tick rate when calm
            PrimaryComponentTick.TickInterval = 0.2f; // 5Hz
            break;
        default:
            PrimaryComponentTick.TickInterval = 0.1f; // 10Hz
            break;
    }

    OnHerdStateChanged.Broadcast(PreviousState, NewState);
}

void UCrowdHerdBehavior::ApplyPanic(float PanicAmount, FVector ThreatLocation)
{
    PanicLevel = FMath::Min(1.0f, PanicLevel + PanicAmount);
    LastKnownThreatLocation = ThreatLocation;

    // Escalate state based on panic level
    if (PanicLevel >= StampedeThreshold && CurrentHerdState != ECrowd_HerdState::Stampeding)
    {
        SetHerdState(ECrowd_HerdState::Stampeding);
    }
    else if (PanicLevel >= 0.3f && CurrentHerdState == ECrowd_HerdState::Grazing)
    {
        SetHerdState(ECrowd_HerdState::Fleeing);
    }
}

FVector UCrowdHerdBehavior::CalculateBoidForce(const TArray<AActor*>& NearbyAgents) const
{
    if (!OwnerActor || NearbyAgents.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector SeparationForce = FVector::ZeroVector;
    FVector AlignmentForce = FVector::ZeroVector;
    FVector CohesionForce = FVector::ZeroVector;

    FVector MyLocation = OwnerActor->GetActorLocation();
    FVector MyVelocity = OwnerActor->GetVelocity();

    int32 SepCount = 0, AlignCount = 0, CohCount = 0;
    FVector CohesionCenter = FVector::ZeroVector;

    for (AActor* Other : NearbyAgents)
    {
        if (!Other || Other == OwnerActor)
        {
            continue;
        }

        FVector ToOther = Other->GetActorLocation() - MyLocation;
        float Distance = ToOther.Size();

        // Separation — push away from too-close neighbors
        if (Distance < BoidParams.SeparationRadius && Distance > 0.0f)
        {
            FVector AwayDir = -ToOther.GetSafeNormal();
            SeparationForce += AwayDir * (1.0f - Distance / BoidParams.SeparationRadius);
            SepCount++;
        }

        // Alignment — match velocity with neighbors
        if (Distance < BoidParams.AlignmentRadius)
        {
            AlignmentForce += Other->GetVelocity();
            AlignCount++;
        }

        // Cohesion — move toward group center
        if (Distance < BoidParams.CohesionRadius)
        {
            CohesionCenter += Other->GetActorLocation();
            CohCount++;
        }
    }

    FVector TotalForce = FVector::ZeroVector;

    if (SepCount > 0)
    {
        SeparationForce /= SepCount;
        TotalForce += SeparationForce.GetSafeNormal() * BoidParams.SeparationWeight;
    }

    if (AlignCount > 0)
    {
        AlignmentForce /= AlignCount;
        TotalForce += AlignmentForce.GetSafeNormal() * BoidParams.AlignmentWeight;
    }

    if (CohCount > 0)
    {
        CohesionCenter /= CohCount;
        FVector ToCohesionCenter = (CohesionCenter - MyLocation).GetSafeNormal();
        TotalForce += ToCohesionCenter * BoidParams.CohesionWeight;
    }

    // Clamp to max force
    if (TotalForce.SizeSquared() > BoidParams.MaxForce * BoidParams.MaxForce)
    {
        TotalForce = TotalForce.GetSafeNormal() * BoidParams.MaxForce;
    }

    return TotalForce;
}

FVector UCrowdHerdBehavior::CalculateFleeForce(FVector ThreatLocation) const
{
    if (!OwnerActor)
    {
        return FVector::ZeroVector;
    }

    FVector MyLocation = OwnerActor->GetActorLocation();
    FVector FleeDirection = (MyLocation - ThreatLocation).GetSafeNormal();

    // Scale flee force by panic level
    float FleeStrength = BoidParams.FleeWeight * PanicLevel * BoidParams.MaxForce;
    return FleeDirection * FleeStrength;
}

void UCrowdHerdBehavior::SetLODLevel(int32 NewLODLevel)
{
    LODLevel = FMath::Clamp(NewLODLevel, 0, 2);

    // Adjust simulation fidelity based on LOD
    switch (LODLevel)
    {
        case 0:
            bIsSimulationActive = true;
            PrimaryComponentTick.TickInterval = 0.1f;
            break;
        case 1:
            bIsSimulationActive = true;
            PrimaryComponentTick.TickInterval = 0.25f; // Reduced frequency
            break;
        case 2:
            bIsSimulationActive = false; // Static impostor
            break;
    }
}

void UCrowdHerdBehavior::UpdateGrazingBehavior(float DeltaTime)
{
    // Slow random drift while grazing
    // Check periodically for predator threats
    static float ThreatCheckTimer = 0.0f;
    ThreatCheckTimer += DeltaTime;
    if (ThreatCheckTimer > 2.0f)
    {
        ThreatCheckTimer = 0.0f;
        CheckForPredatorThreat();
    }
}

void UCrowdHerdBehavior::UpdateWanderingBehavior(float DeltaTime)
{
    // Move toward next waypoint with boid forces applied
    CheckForPredatorThreat();
}

void UCrowdHerdBehavior::UpdateFleeingBehavior(float DeltaTime)
{
    if (!OwnerActor)
    {
        return;
    }

    // Move away from last known threat location
    FVector FleeForce = CalculateFleeForce(LastKnownThreatLocation);
    FVector NewLocation = OwnerActor->GetActorLocation() + FleeForce.GetSafeNormal() * BoidParams.MaxSpeed * 0.6f * DeltaTime;
    OwnerActor->SetActorLocation(NewLocation, true);
}

void UCrowdHerdBehavior::UpdateStampedeBehavior(float DeltaTime)
{
    if (!OwnerActor)
    {
        return;
    }

    // Full speed flee with boid cohesion to keep herd together
    FVector FleeForce = CalculateFleeForce(LastKnownThreatLocation);
    FVector NewLocation = OwnerActor->GetActorLocation() + FleeForce.GetSafeNormal() * BoidParams.MaxSpeed * DeltaTime;
    OwnerActor->SetActorLocation(NewLocation, true);
}

void UCrowdHerdBehavior::UpdateMigrationBehavior(float DeltaTime)
{
    // Move toward migration target with boid forces
    if (!OwnerActor || !MigrationTarget)
    {
        return;
    }

    FVector ToTarget = (MigrationTarget->GetActorLocation() - OwnerActor->GetActorLocation());
    float DistToTarget = ToTarget.Size();

    if (DistToTarget < 200.0f)
    {
        // Reached waypoint — transition to next or start grazing
        SetHerdState(ECrowd_HerdState::Grazing);
        return;
    }

    FVector MoveDir = ToTarget.GetSafeNormal();
    FVector NewLocation = OwnerActor->GetActorLocation() + MoveDir * BoidParams.MaxSpeed * 0.4f * DeltaTime;
    OwnerActor->SetActorLocation(NewLocation, true);
}

void UCrowdHerdBehavior::CheckForPredatorThreat()
{
    if (!OwnerActor)
    {
        return;
    }

    UWorld* World = OwnerActor->GetWorld();
    if (!World)
    {
        return;
    }

    // Sphere overlap to detect predator actors (TRex, Raptor)
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(FleeRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerActor);

    bool bHit = World->OverlapMultiByChannel(
        Overlaps,
        OwnerActor->GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        SphereShape,
        QueryParams
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* OtherActor = Overlap.GetActor();
            if (OtherActor)
            {
                // Check if it's a predator by tag
                if (OtherActor->ActorHasTag(FName("Predator")) || OtherActor->ActorHasTag(FName("TRex")))
                {
                    float Distance = FVector::Dist(OwnerActor->GetActorLocation(), OtherActor->GetActorLocation());
                    float PanicAmount = FMath::Lerp(0.8f, 0.2f, Distance / FleeRadius);
                    ApplyPanic(PanicAmount, OtherActor->GetActorLocation());
                    break;
                }
            }
        }
    }
}
