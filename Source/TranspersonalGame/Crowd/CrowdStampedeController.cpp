// CrowdStampedeController.cpp
// Agent #13 — Crowd & Traffic Simulation
// Stampede controller: manages herd state transitions, boid flocking, threat response

#include "CrowdStampedeController.h"
#include "CrowdSimulationTypes.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UCrowdStampedeController::UCrowdStampedeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update for performance

    HerdState = ECrowd_HerdState::Grazing;
    CurrentThreatLevel = ECrowd_ThreatLevel::None;
    bStampedeActive = false;
    StampedeTimer = 0.0f;
    AlertTimer = 0.0f;
    HerdCenterLocation = FVector::ZeroVector;
}

void UCrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();
    InitializeHerd();
}

void UCrowdStampedeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateHerdCenter();
    UpdateHerdStateMachine(DeltaTime);
    ApplyBoidFlocking(DeltaTime);

    if (bStampedeActive)
    {
        StampedeTimer -= DeltaTime;
        if (StampedeTimer <= 0.0f)
        {
            EndStampede();
        }
    }

    if (HerdState == ECrowd_HerdState::Alerted)
    {
        AlertTimer -= DeltaTime;
        if (AlertTimer <= 0.0f)
        {
            TransitionToState(ECrowd_HerdState::Wandering);
        }
    }
}

void UCrowdStampedeController::InitializeHerd()
{
    HerdMembers.Empty();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector OwnerLocation = Owner->GetActorLocation();

    for (int32 i = 0; i < HerdConfig.HerdSize; i++)
    {
        FCrowd_HerdMember Member;
        Member.AgentID = i;
        Member.bIsAlpha = (i == 0);
        Member.Role = (i == 0) ? ECrowd_AgentRole::AlphaLeader :
                      (i < 3)  ? ECrowd_AgentRole::Scout :
                                 ECrowd_AgentRole::Follower;
        Member.Species = HerdConfig.Species;
        Member.Health = 100.0f;
        Member.FearLevel = 0.0f;

        // Distribute members in a loose circle
        float Angle = (float)i / (float)HerdConfig.HerdSize * 2.0f * PI;
        float Radius = HerdConfig.SeparationDistance * 2.0f;
        Member.Location = OwnerLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        Member.Velocity = FVector::ZeroVector;

        HerdMembers.Add(Member);
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Initialized herd with %d members"), HerdMembers.Num());
}

void UCrowdStampedeController::UpdateHerdCenter()
{
    if (HerdMembers.Num() == 0) return;

    FVector Sum = FVector::ZeroVector;
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        Sum += Member.Location;
    }
    HerdCenterLocation = Sum / (float)HerdMembers.Num();
}

void UCrowdStampedeController::UpdateHerdStateMachine(float DeltaTime)
{
    switch (HerdState)
    {
        case ECrowd_HerdState::Grazing:
            UpdateGrazingBehavior(DeltaTime);
            break;
        case ECrowd_HerdState::Wandering:
            UpdateWanderingBehavior(DeltaTime);
            break;
        case ECrowd_HerdState::Alerted:
            UpdateAlertedBehavior(DeltaTime);
            break;
        case ECrowd_HerdState::Stampeding:
            UpdateStampedeBehavior(DeltaTime);
            break;
        case ECrowd_HerdState::Fleeing:
            UpdateFleeingBehavior(DeltaTime);
            break;
        case ECrowd_HerdState::Resting:
            // Minimal movement — just maintain formation
            break;
    }
}

void UCrowdStampedeController::UpdateGrazingBehavior(float DeltaTime)
{
    // Slow random drift while grazing
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        FVector RandomDrift = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal() * HerdConfig.GrazeSpeed * 0.1f;

        Member.Velocity = FMath::VInterpTo(Member.Velocity, RandomDrift, DeltaTime, 0.5f);
        Member.Location += Member.Velocity * DeltaTime;
    }
}

void UCrowdStampedeController::UpdateWanderingBehavior(float DeltaTime)
{
    if (HerdMembers.Num() == 0) return;

    // Alpha leads, others follow
    FCrowd_HerdMember& Alpha = HerdMembers[0];
    FVector AlphaTarget = Alpha.Location + Alpha.Velocity.GetSafeNormal() * HerdConfig.WanderRadius * 0.1f;

    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        FCrowd_HerdMember& Member = HerdMembers[i];
        FVector TargetVelocity;

        if (Member.bIsAlpha)
        {
            TargetVelocity = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal() * HerdConfig.WalkSpeed;
        }
        else
        {
            // Follow alpha with offset
            FVector ToAlpha = (Alpha.Location - Member.Location).GetSafeNormal();
            TargetVelocity = ToAlpha * HerdConfig.WalkSpeed;
        }

        Member.Velocity = FMath::VInterpTo(Member.Velocity, TargetVelocity, DeltaTime, 1.5f);
        Member.Location += Member.Velocity * DeltaTime;
    }
}

void UCrowdStampedeController::UpdateAlertedBehavior(float DeltaTime)
{
    // Face threat, tighten formation
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        FVector ToCenter = (HerdCenterLocation - Member.Location);
        float DistToCenter = ToCenter.Size();

        if (DistToCenter > HerdConfig.SeparationDistance * 1.5f)
        {
            Member.Velocity = FMath::VInterpTo(Member.Velocity, ToCenter.GetSafeNormal() * HerdConfig.WalkSpeed * 0.5f, DeltaTime, 2.0f);
            Member.Location += Member.Velocity * DeltaTime;
        }
        else
        {
            Member.Velocity = FMath::VInterpTo(Member.Velocity, FVector::ZeroVector, DeltaTime, 3.0f);
        }

        Member.FearLevel = FMath::FInterpTo(Member.FearLevel, 0.5f, DeltaTime, 1.0f);
    }
}

void UCrowdStampedeController::UpdateStampedeBehavior(float DeltaTime)
{
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        // All members run in stampede direction at full speed
        FVector StampedeVelocity = ActiveStampedeData.StampedeDirection * ActiveStampedeData.StampedeSpeed;

        // Add slight lateral variation per member
        float LateralOffset = FMath::Sin((float)Member.AgentID * 0.7f + StampedeTimer) * 100.0f;
        StampedeVelocity += FVector(-ActiveStampedeData.StampedeDirection.Y, ActiveStampedeData.StampedeDirection.X, 0.0f) * LateralOffset;

        Member.Velocity = FMath::VInterpTo(Member.Velocity, StampedeVelocity, DeltaTime, 3.0f);
        Member.Location += Member.Velocity * DeltaTime;
        Member.FearLevel = 1.0f;
    }
}

void UCrowdStampedeController::UpdateFleeingBehavior(float DeltaTime)
{
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        FVector FleeDirection = (Member.Location - ThreatLocation).GetSafeNormal();
        FVector FleeVelocity = FleeDirection * HerdConfig.FleeSpeed;

        Member.Velocity = FMath::VInterpTo(Member.Velocity, FleeVelocity, DeltaTime, 2.5f);
        Member.Location += Member.Velocity * DeltaTime;
        Member.FearLevel = FMath::FInterpTo(Member.FearLevel, 0.8f, DeltaTime, 2.0f);
    }
}

void UCrowdStampedeController::ApplyBoidFlocking(float DeltaTime)
{
    // Boid rules: separation, cohesion, alignment
    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        FCrowd_HerdMember& Member = HerdMembers[i];
        FVector Separation = FVector::ZeroVector;
        FVector Cohesion = FVector::ZeroVector;
        FVector Alignment = FVector::ZeroVector;
        int32 NeighborCount = 0;

        for (int32 j = 0; j < HerdMembers.Num(); j++)
        {
            if (i == j) continue;

            const FCrowd_HerdMember& Other = HerdMembers[j];
            FVector Diff = Member.Location - Other.Location;
            float Dist = Diff.Size();

            if (Dist < HerdConfig.SeparationDistance)
            {
                // Separation: push away from close neighbors
                Separation += Diff.GetSafeNormal() / FMath::Max(Dist, 1.0f);
            }

            if (Dist < HerdConfig.SeparationDistance * 3.0f)
            {
                Cohesion += Other.Location;
                Alignment += Other.Velocity;
                NeighborCount++;
            }
        }

        if (NeighborCount > 0)
        {
            Cohesion = (Cohesion / (float)NeighborCount - Member.Location).GetSafeNormal();
            Alignment = (Alignment / (float)NeighborCount).GetSafeNormal();

            FVector FlockForce = Separation * HerdConfig.SeparationWeight
                               + Cohesion   * HerdConfig.CohesionWeight
                               + Alignment  * HerdConfig.AlignmentWeight;

            Member.Velocity += FlockForce * DeltaTime * 50.0f;

            // Clamp velocity based on state
            float MaxSpeed = (HerdState == ECrowd_HerdState::Stampeding) ? ActiveStampedeData.StampedeSpeed
                           : (HerdState == ECrowd_HerdState::Fleeing)    ? HerdConfig.FleeSpeed
                           : (HerdState == ECrowd_HerdState::Wandering)  ? HerdConfig.WalkSpeed
                           :                                                HerdConfig.GrazeSpeed;

            if (Member.Velocity.Size() > MaxSpeed)
            {
                Member.Velocity = Member.Velocity.GetSafeNormal() * MaxSpeed;
            }
        }
    }
}

void UCrowdStampedeController::TriggerStampede(FVector TriggerLocation, FVector Direction)
{
    if (bStampedeActive) return;

    ActiveStampedeData.StampedeTriggerLocation = TriggerLocation;
    ActiveStampedeData.StampedeDirection = Direction.GetSafeNormal();
    ActiveStampedeData.bIsActive = true;
    ActiveStampedeData.ThreatLevel = ECrowd_ThreatLevel::Critical;
    StampedeTimer = ActiveStampedeData.StampedeDuration;
    bStampedeActive = true;
    ThreatLocation = TriggerLocation;

    TransitionToState(ECrowd_HerdState::Stampeding);

    UE_LOG(LogTemp, Warning, TEXT("CrowdStampedeController: STAMPEDE TRIGGERED at %s, direction %s"),
        *TriggerLocation.ToString(), *Direction.ToString());
}

void UCrowdStampedeController::EndStampede()
{
    bStampedeActive = false;
    ActiveStampedeData.bIsActive = false;
    StampedeTimer = 0.0f;

    TransitionToState(ECrowd_HerdState::Wandering);
    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Stampede ended — herd transitioning to Wandering"));
}

void UCrowdStampedeController::AlertHerd(FVector ThreatPos, ECrowd_ThreatLevel Threat)
{
    ThreatLocation = ThreatPos;
    CurrentThreatLevel = Threat;

    switch (Threat)
    {
        case ECrowd_ThreatLevel::Critical:
        case ECrowd_ThreatLevel::High:
        {
            FVector FleeDir = (HerdCenterLocation - ThreatPos).GetSafeNormal();
            TriggerStampede(ThreatPos, FleeDir);
            break;
        }
        case ECrowd_ThreatLevel::Medium:
            TransitionToState(ECrowd_HerdState::Fleeing);
            break;
        case ECrowd_ThreatLevel::Low:
            TransitionToState(ECrowd_HerdState::Alerted);
            AlertTimer = 8.0f;
            break;
        default:
            break;
    }

    // Propagate fear to all members
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        float FearIncrease = (Threat == ECrowd_ThreatLevel::Critical) ? 1.0f :
                             (Threat == ECrowd_ThreatLevel::High)     ? 0.8f :
                             (Threat == ECrowd_ThreatLevel::Medium)   ? 0.5f : 0.2f;
        Member.FearLevel = FMath::Clamp(Member.FearLevel + FearIncrease, 0.0f, 1.0f);
    }
}

void UCrowdStampedeController::TransitionToState(ECrowd_HerdState NewState)
{
    if (HerdState == NewState) return;

    ECrowd_HerdState OldState = HerdState;
    HerdState = NewState;

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: State %d -> %d"),
        (int32)OldState, (int32)NewState);
}

int32 UCrowdStampedeController::GetHerdSize() const
{
    return HerdMembers.Num();
}

FVector UCrowdStampedeController::GetHerdCenter() const
{
    return HerdCenterLocation;
}

ECrowd_HerdState UCrowdStampedeController::GetHerdState() const
{
    return HerdState;
}

TArray<FCrowd_HerdMember> UCrowdStampedeController::GetHerdMembers() const
{
    return HerdMembers;
}
