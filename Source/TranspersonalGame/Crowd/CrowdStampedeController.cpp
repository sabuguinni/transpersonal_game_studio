#include "CrowdStampedeController.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UCrowd_StampedeControllerComponent
// ============================================================

UCrowd_StampedeControllerComponent::UCrowd_StampedeControllerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz tick for performance
    StampedeData.State = ECrowd_StampedeState::Idle;
    CurrentPanicLevel = 0.0f;
}

void UCrowd_StampedeControllerComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCrowd_StampedeControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (StampedeData.State == ECrowd_StampedeState::Idle || Agents.Num() == 0)
    {
        return;
    }

    StampedeData.ElapsedTime += DeltaTime;
    PropagateStampedePanic(DeltaTime);
    UpdateStampedePhysics(DeltaTime);

    // Transition to Dispersing after duration
    if (StampedeData.State == ECrowd_StampedeState::Running &&
        StampedeData.ElapsedTime >= StampedeData.StampedeDuration)
    {
        StampedeData.State = ECrowd_StampedeState::Dispersing;
    }

    // Transition to Settled when panic is low
    if (StampedeData.State == ECrowd_StampedeState::Dispersing && CurrentPanicLevel < 0.1f)
    {
        StampedeData.State = ECrowd_StampedeState::Settled;
        StampedeData.ElapsedTime = 0.0f;
    }
}

void UCrowd_StampedeControllerComponent::TriggerStampede(ECrowd_StampedeTrigger InTrigger, FVector InTriggerLocation)
{
    StampedeData.Trigger = InTrigger;
    StampedeData.TriggerLocation = InTriggerLocation;
    StampedeData.State = ECrowd_StampedeState::Gathering;
    StampedeData.ElapsedTime = 0.0f;
    CurrentPanicLevel = 1.0f;

    // Compute flee direction — away from trigger
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector OwnerLoc = Owner->GetActorLocation();
        FVector AwayDir = (OwnerLoc - InTriggerLocation).GetSafeNormal();
        StampedeData.RunDirection = AwayDir;
    }

    // Short gathering phase, then Running
    StampedeData.State = ECrowd_StampedeState::Running;

    UE_LOG(LogTemp, Warning, TEXT("[Crowd] Stampede triggered! Trigger=%d, PanicLevel=1.0, Agents=%d"),
        (int32)InTrigger, Agents.Num());
}

void UCrowd_StampedeControllerComponent::StopStampede()
{
    StampedeData.State = ECrowd_StampedeState::Dispersing;
    CurrentPanicLevel = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("[Crowd] Stampede stopped manually."));
}

void UCrowd_StampedeControllerComponent::RegisterAgent(int32 AgentID, FVector InitialLocation, float AgentMass)
{
    FCrowd_StampedeAgent NewAgent;
    NewAgent.AgentID = AgentID;
    NewAgent.Location = InitialLocation;
    NewAgent.Mass = AgentMass;
    NewAgent.Speed = FMath::RandRange(500.0f, MaxStampedeSpeed);
    NewAgent.bIsLeader = (Agents.Num() == 0); // First agent is leader
    Agents.Add(NewAgent);
    StampedeData.AgentCount = Agents.Num();
}

void UCrowd_StampedeControllerComponent::UnregisterAgent(int32 AgentID)
{
    Agents.RemoveAll([AgentID](const FCrowd_StampedeAgent& A) { return A.AgentID == AgentID; });
    StampedeData.AgentCount = Agents.Num();
}

FVector UCrowd_StampedeControllerComponent::GetFleeDirection(FVector AgentLocation) const
{
    if (StampedeData.State == ECrowd_StampedeState::Idle)
    {
        return FVector::ZeroVector;
    }

    FVector AwayFromTrigger = (AgentLocation - StampedeData.TriggerLocation).GetSafeNormal();
    FVector BlendedDir = FMath::Lerp(StampedeData.RunDirection, AwayFromTrigger, 0.3f);
    return BlendedDir.GetSafeNormal();
}

float UCrowd_StampedeControllerComponent::GetPanicIntensity(FVector AgentLocation) const
{
    float DistToTrigger = FVector::Dist(AgentLocation, StampedeData.TriggerLocation);
    float NormalizedDist = FMath::Clamp(DistToTrigger / StampedeData.PanicRadius, 0.0f, 1.0f);
    float DistanceFactor = 1.0f - NormalizedDist;
    return FMath::Clamp(CurrentPanicLevel * DistanceFactor, 0.0f, 1.0f);
}

void UCrowd_StampedeControllerComponent::PropagateStampedePanic(float DeltaTime)
{
    if (StampedeData.State == ECrowd_StampedeState::Running)
    {
        // Panic sustains during running, slight decay
        CurrentPanicLevel = FMath::Clamp(CurrentPanicLevel - (PanicDecayRate * 0.1f * DeltaTime), 0.0f, 1.0f);
    }
    else if (StampedeData.State == ECrowd_StampedeState::Dispersing)
    {
        // Faster decay during dispersal
        CurrentPanicLevel = FMath::Clamp(CurrentPanicLevel - (PanicDecayRate * DeltaTime), 0.0f, 1.0f);
    }
}

void UCrowd_StampedeControllerComponent::UpdateStampedePhysics(float DeltaTime)
{
    if (StampedeData.State != ECrowd_StampedeState::Running &&
        StampedeData.State != ECrowd_StampedeState::Dispersing)
    {
        return;
    }

    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        FVector FleeDir = GetFleeDirection(Agent.Location);
        FVector Separation = ComputeSeparationForce(Agent);
        FVector Alignment = ComputeAlignmentForce(Agent);
        FVector Cohesion = ComputeCohesionForce(Agent);

        // Weighted flocking: flee direction dominates, flocking adds realism
        FVector DesiredVelocity = (FleeDir * 0.6f) + (Separation * 0.2f) + (Alignment * 0.1f) + (Cohesion * 0.1f);
        DesiredVelocity = DesiredVelocity.GetSafeNormal() * Agent.Speed * CurrentPanicLevel;

        // Smooth velocity
        Agent.Velocity = FMath::VInterpTo(Agent.Velocity, DesiredVelocity, DeltaTime, 3.0f);

        // Update position
        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

FVector UCrowd_StampedeControllerComponent::ComputeSeparationForce(const FCrowd_StampedeAgent& Agent) const
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_StampedeAgent& Other : Agents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < SeparationRadius && Dist > 0.0f)
        {
            FVector Away = (Agent.Location - Other.Location).GetSafeNormal();
            SeparationForce += Away / Dist; // Stronger when closer
            Count++;
        }
    }

    return Count > 0 ? (SeparationForce / Count).GetSafeNormal() : FVector::ZeroVector;
}

FVector UCrowd_StampedeControllerComponent::ComputeAlignmentForce(const FCrowd_StampedeAgent& Agent) const
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_StampedeAgent& Other : Agents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < AlignmentRadius)
        {
            AvgVelocity += Other.Velocity;
            Count++;
        }
    }

    return Count > 0 ? (AvgVelocity / Count).GetSafeNormal() : FVector::ZeroVector;
}

FVector UCrowd_StampedeControllerComponent::ComputeCohesionForce(const FCrowd_StampedeAgent& Agent) const
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_StampedeAgent& Other : Agents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < CohesionRadius)
        {
            CenterOfMass += Other.Location;
            Count++;
        }
    }

    if (Count > 0)
    {
        CenterOfMass /= Count;
        return (CenterOfMass - Agent.Location).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

// ============================================================
// ACrowd_StampedeActor
// ============================================================

ACrowd_StampedeActor::ACrowd_StampedeActor()
{
    PrimaryActorTick.bCanEverTick = true;
    StampedeController = CreateDefaultSubobject<UCrowd_StampedeControllerComponent>(TEXT("StampedeController"));
}

void ACrowd_StampedeActor::BeginPlay()
{
    Super::BeginPlay();
    InitializeHerd();
}

void ACrowd_StampedeActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACrowd_StampedeActor::InitializeHerd()
{
    if (!StampedeController) return;

    FVector HerdCenter = GetActorLocation();

    for (int32 i = 0; i < HerdSize; i++)
    {
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Radius = FMath::RandRange(0.0f, HerdRadius);
        FVector AgentLoc = HerdCenter + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
            0.0f
        );

        float AgentMass = FMath::RandRange(300.0f, 800.0f);
        StampedeController->RegisterAgent(i, AgentLoc, AgentMass);
    }

    UE_LOG(LogTemp, Log, TEXT("[Crowd] Herd '%s' initialized with %d agents of species '%s'"),
        *GetName(), HerdSize, *HerdSpecies);
}

void ACrowd_StampedeActor::OnPredatorDetected(AActor* Predator)
{
    if (!Predator || !StampedeController) return;

    FVector PredatorLoc = Predator->GetActorLocation();
    StampedeController->TriggerStampede(ECrowd_StampedeTrigger::PredatorNearby, PredatorLoc);

    UE_LOG(LogTemp, Warning, TEXT("[Crowd] Predator detected by herd '%s'! Stampede triggered from location %s"),
        *GetName(), *PredatorLoc.ToString());
}
