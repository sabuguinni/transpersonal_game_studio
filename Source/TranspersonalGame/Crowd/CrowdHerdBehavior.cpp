// CrowdHerdBehavior.cpp
// Agent #13 — Crowd & Traffic Simulation
// Boids flocking algorithm + herd state machine for prehistoric herbivore herds.
// Supports up to 200 agents with separation, alignment, cohesion forces.

#include "CrowdHerdBehavior.h"
#include "Math/UnrealMathUtility.h"

UCrowdHerdBehavior::UCrowdHerdBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz tick for crowd sim
}

void UCrowdHerdBehavior::BeginPlay()
{
    Super::BeginPlay();
}

void UCrowdHerdBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (HerdMembers.Num() == 0) return;

    UpdateHerdCenter();
    UpdatePanicLevels(DeltaTime);
    EvaluateHerdStateTransition();

    // Update velocities using Boids for Wandering/Fleeing/Stampeding states
    if (CurrentHerdState == ECrowd_HerdState::Wandering ||
        CurrentHerdState == ECrowd_HerdState::Fleeing ||
        CurrentHerdState == ECrowd_HerdState::Stampeding)
    {
        for (FCrowd_HerdMember& Member : HerdMembers)
        {
            if (!Member.bIsAlive) continue;

            FVector SteeringForce = ComputeBoidForce(Member);
            Member.Velocity += SteeringForce * DeltaTime;
            Member.Velocity = LimitVector(Member.Velocity, BoidParams.MaxSpeed);
            Member.Location += Member.Velocity * DeltaTime;
        }
    }
}

void UCrowdHerdBehavior::InitializeHerd(int32 NumMembers, FVector SpawnCenter, float SpawnRadius)
{
    HerdMembers.Empty();
    int32 ClampedNum = FMath::Clamp(NumMembers, 1, MaxHerdSize);

    for (int32 i = 0; i < ClampedNum; ++i)
    {
        FCrowd_HerdMember Member;
        Member.AgentID = i;
        Member.bIsAlive = true;
        Member.PanicLevel = 0.0f;

        // Random position within spawn radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Dist = FMath::RandRange(0.0f, SpawnRadius);
        Member.Location = SpawnCenter + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.0f);

        // Random initial velocity
        Member.Velocity = FVector(FMath::RandRange(-100.0f, 100.0f), FMath::RandRange(-100.0f, 100.0f), 0.0f);

        // Assign roles: 1 leader, 10% scouts, rest followers
        if (i == 0)
            Member.Role = ECrowd_HerdRole::Leader;
        else if (i < FMath::Max(1, ClampedNum / 10))
            Member.Role = ECrowd_HerdRole::Scout;
        else if (i >= ClampedNum - FMath::Max(1, ClampedNum / 5))
            Member.Role = ECrowd_HerdRole::Juvenile;
        else
            Member.Role = ECrowd_HerdRole::Follower;

        HerdMembers.Add(Member);
    }

    UpdateHerdCenter();
    UE_LOG(LogTemp, Log, TEXT("CrowdHerdBehavior: Initialized herd with %d members at %s"), ClampedNum, *SpawnCenter.ToString());
}

void UCrowdHerdBehavior::ApplyThreatAtLocation(FVector ThreatLocation, float ThreatRadius, float PanicAmount)
{
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.bIsAlive) continue;

        float Dist = FVector::Dist(Member.Location, ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            float DistFactor = 1.0f - (Dist / ThreatRadius);
            float AppliedPanic = PanicAmount * DistFactor;
            Member.PanicLevel = FMath::Clamp(Member.PanicLevel + AppliedPanic, 0.0f, 1.0f);
        }
    }
    EvaluateHerdStateTransition();
}

void UCrowdHerdBehavior::SetHerdState(ECrowd_HerdState NewState)
{
    if (CurrentHerdState != NewState)
    {
        UE_LOG(LogTemp, Log, TEXT("CrowdHerdBehavior: Herd state transition %d -> %d"), (int32)CurrentHerdState, (int32)NewState);
        CurrentHerdState = NewState;
    }
}

FVector UCrowdHerdBehavior::ComputeBoidForce(const FCrowd_HerdMember& Agent) const
{
    FVector Separation = ComputeSeparation(Agent) * BoidParams.SeparationWeight;
    FVector Alignment = ComputeAlignment(Agent) * BoidParams.AlignmentWeight;
    FVector Cohesion = ComputeCohesion(Agent) * BoidParams.CohesionWeight;

    // In panic/stampede, separation and flee dominate
    if (CurrentHerdState == ECrowd_HerdState::Stampeding)
    {
        Separation *= 2.0f;
        Alignment *= 1.5f;
    }

    FVector TotalForce = Separation + Alignment + Cohesion;
    return LimitVector(TotalForce, BoidParams.MaxForce);
}

FVector UCrowdHerdBehavior::GetHerdFleeDirection(FVector ThreatLocation) const
{
    FVector FleeDir = (HerdCenter - ThreatLocation);
    FleeDir.Z = 0.0f;
    return FleeDir.GetSafeNormal();
}

int32 UCrowdHerdBehavior::GetAliveCount() const
{
    int32 Count = 0;
    for (const FCrowd_HerdMember& M : HerdMembers)
    {
        if (M.bIsAlive) Count++;
    }
    return Count;
}

void UCrowdHerdBehavior::KillMember(int32 AgentID)
{
    for (FCrowd_HerdMember& M : HerdMembers)
    {
        if (M.AgentID == AgentID)
        {
            M.bIsAlive = false;
            UE_LOG(LogTemp, Log, TEXT("CrowdHerdBehavior: Agent %d killed — herd size now %d"), AgentID, GetAliveCount());
            return;
        }
    }
}

bool UCrowdHerdBehavior::IsHerdInPanic() const
{
    return AveragePanicLevel >= PanicThresholdFlee;
}

// ---- Private ----

void UCrowdHerdBehavior::UpdateHerdCenter()
{
    FVector Sum = FVector::ZeroVector;
    int32 AliveCount = 0;
    for (const FCrowd_HerdMember& M : HerdMembers)
    {
        if (M.bIsAlive)
        {
            Sum += M.Location;
            AliveCount++;
        }
    }
    HerdCenter = (AliveCount > 0) ? (Sum / (float)AliveCount) : FVector::ZeroVector;
}

void UCrowdHerdBehavior::UpdatePanicLevels(float DeltaTime)
{
    float TotalPanic = 0.0f;
    int32 AliveCount = 0;

    for (FCrowd_HerdMember& M : HerdMembers)
    {
        if (!M.bIsAlive) continue;

        // Panic decays over time
        M.PanicLevel = FMath::Clamp(M.PanicLevel - PanicDecayRate * DeltaTime, 0.0f, 1.0f);
        TotalPanic += M.PanicLevel;
        AliveCount++;
    }

    AveragePanicLevel = (AliveCount > 0) ? (TotalPanic / (float)AliveCount) : 0.0f;
}

void UCrowdHerdBehavior::EvaluateHerdStateTransition()
{
    if (AveragePanicLevel >= PanicThresholdStampede)
    {
        SetHerdState(ECrowd_HerdState::Stampeding);
    }
    else if (AveragePanicLevel >= PanicThresholdFlee)
    {
        SetHerdState(ECrowd_HerdState::Fleeing);
    }
    else if (AveragePanicLevel < 0.05f)
    {
        // Calm — return to grazing or wandering
        if (CurrentHerdState == ECrowd_HerdState::Fleeing || CurrentHerdState == ECrowd_HerdState::Stampeding)
        {
            SetHerdState(ECrowd_HerdState::Wandering);
        }
    }
}

FVector UCrowdHerdBehavior::ComputeSeparation(const FCrowd_HerdMember& Agent) const
{
    FVector SteeringForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (!Other.bIsAlive || Other.AgentID == Agent.AgentID) continue;

        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist > 0.0f && Dist < BoidParams.SeparationRadius)
        {
            FVector Diff = (Agent.Location - Other.Location).GetSafeNormal();
            Diff /= Dist; // Weight by distance
            SteeringForce += Diff;
            Count++;
        }
    }

    if (Count > 0)
    {
        SteeringForce /= (float)Count;
        SteeringForce = LimitVector(SteeringForce, BoidParams.MaxSpeed);
        SteeringForce -= Agent.Velocity;
        SteeringForce = LimitVector(SteeringForce, BoidParams.MaxForce);
    }

    return SteeringForce;
}

FVector UCrowdHerdBehavior::ComputeAlignment(const FCrowd_HerdMember& Agent) const
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (!Other.bIsAlive || Other.AgentID == Agent.AgentID) continue;

        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < BoidParams.AlignmentRadius)
        {
            AvgVelocity += Other.Velocity;
            Count++;
        }
    }

    if (Count > 0)
    {
        AvgVelocity /= (float)Count;
        AvgVelocity = LimitVector(AvgVelocity, BoidParams.MaxSpeed);
        FVector Steering = AvgVelocity - Agent.Velocity;
        return LimitVector(Steering, BoidParams.MaxForce);
    }

    return FVector::ZeroVector;
}

FVector UCrowdHerdBehavior::ComputeCohesion(const FCrowd_HerdMember& Agent) const
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (!Other.bIsAlive || Other.AgentID == Agent.AgentID) continue;

        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < BoidParams.CohesionRadius)
        {
            CenterOfMass += Other.Location;
            Count++;
        }
    }

    if (Count > 0)
    {
        CenterOfMass /= (float)Count;
        FVector Desired = (CenterOfMass - Agent.Location).GetSafeNormal() * BoidParams.MaxSpeed;
        FVector Steering = Desired - Agent.Velocity;
        return LimitVector(Steering, BoidParams.MaxForce);
    }

    return FVector::ZeroVector;
}

FVector UCrowdHerdBehavior::LimitVector(FVector V, float MaxMagnitude) const
{
    float Mag = V.Size();
    if (Mag > MaxMagnitude && Mag > 0.0f)
    {
        return V * (MaxMagnitude / Mag);
    }
    return V;
}
