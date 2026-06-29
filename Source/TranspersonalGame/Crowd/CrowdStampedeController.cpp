#include "CrowdStampedeController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

ACrowd_StampedeController::ACrowd_StampedeController()
{
    PrimaryActorTick.bCanEverTick = true;

    CurrentState = ECrowd_StampedeState::Idle;
    MaxStampedeSpeed = 1200.0f;
    FearDecayRate = 0.05f;
    DangerZoneRadius = 600.0f;
    SeparationForce = 1.5f;
    CohesionForce = 0.8f;
    AlignmentForce = 1.0f;
}

void ACrowd_StampedeController::BeginPlay()
{
    Super::BeginPlay();
}

void ACrowd_StampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (CurrentState)
    {
    case ECrowd_StampedeState::Alarmed:
    case ECrowd_StampedeState::Fleeing:
    case ECrowd_StampedeState::Stampeding:
        UpdateStampedeMovement(DeltaTime);
        UpdateFlockingBehavior(DeltaTime);
        break;

    case ECrowd_StampedeState::Dispersed:
        CalmHerd(FearDecayRate * DeltaTime);
        if (AverageFearLevel() < 0.05f)
        {
            CurrentState = ECrowd_StampedeState::Idle;
        }
        break;

    default:
        break;
    }
}

void ACrowd_StampedeController::TriggerStampede(FVector TriggerLocation, FVector FleeDirection, float PanicRadius)
{
    StampedeData.TriggerLocation = TriggerLocation;
    StampedeData.FleeDirection = FleeDirection.GetSafeNormal();
    StampedeData.PanicRadius = PanicRadius;

    // Set fear on all members within panic radius
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        float Dist = FVector::Dist(Member.Location, TriggerLocation);
        if (Dist <= PanicRadius)
        {
            float FearIntensity = 1.0f - (Dist / PanicRadius);
            Member.FearLevel = FMath::Max(Member.FearLevel, FearIntensity);
        }
    }

    float AvgFear = AverageFearLevel();
    if (AvgFear > 0.7f)
    {
        CurrentState = ECrowd_StampedeState::Stampeding;
    }
    else if (AvgFear > 0.3f)
    {
        CurrentState = ECrowd_StampedeState::Fleeing;
    }
    else if (AvgFear > 0.1f)
    {
        CurrentState = ECrowd_StampedeState::Alarmed;
    }
}

void ACrowd_StampedeController::AddHerdMember(FVector Location, bool bIsLeader)
{
    FCrowd_HerdMember NewMember;
    NewMember.Location = Location;
    NewMember.Velocity = FVector::ZeroVector;
    NewMember.FearLevel = 0.0f;
    NewMember.bIsLeader = bIsLeader;
    HerdMembers.Add(NewMember);
}

void ACrowd_StampedeController::CalmHerd(float CalmRate)
{
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        Member.FearLevel = FMath::Max(0.0f, Member.FearLevel - CalmRate);
    }
}

bool ACrowd_StampedeController::IsPlayerInDangerZone(FVector PlayerLocation) const
{
    if (CurrentState != ECrowd_StampedeState::Stampeding)
    {
        return false;
    }

    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        float Dist = FVector::Dist(Member.Location, PlayerLocation);
        if (Dist <= DangerZoneRadius)
        {
            return true;
        }
    }
    return false;
}

void ACrowd_StampedeController::UpdateFlockingBehavior(float DeltaTime)
{
    for (int32 i = 0; i < HerdMembers.Num(); ++i)
    {
        FCrowd_HerdMember& Member = HerdMembers[i];

        FVector Separation = CalculateSeparation(i) * SeparationForce;
        FVector Cohesion = CalculateCohesion(i) * CohesionForce;
        FVector Alignment = CalculateAlignment(i) * AlignmentForce;

        // Fear-weighted flee direction
        FVector FleeForce = StampedeData.FleeDirection * (Member.FearLevel * MaxStampedeSpeed);

        FVector TotalForce = Separation + Cohesion + Alignment + FleeForce;
        Member.Velocity = FMath::VInterpTo(Member.Velocity, TotalForce, DeltaTime, 3.0f);

        float Speed = FMath::Lerp(200.0f, MaxStampedeSpeed, Member.FearLevel);
        Member.Velocity = Member.Velocity.GetClampedToMaxSize(Speed);
    }
}

void ACrowd_StampedeController::UpdateStampedeMovement(float DeltaTime)
{
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        Member.Location += Member.Velocity * DeltaTime;
        // Decay fear gradually during movement
        Member.FearLevel = FMath::Max(0.0f, Member.FearLevel - (FearDecayRate * DeltaTime * 0.1f));
    }

    // Check if herd has dispersed
    if (AverageFearLevel() < 0.2f && CurrentState == ECrowd_StampedeState::Stampeding)
    {
        CurrentState = ECrowd_StampedeState::Dispersed;
    }
}

FVector ACrowd_StampedeController::CalculateSeparation(int32 MemberIndex) const
{
    const FCrowd_HerdMember& Self = HerdMembers[MemberIndex];
    FVector SeparationVec = FVector::ZeroVector;
    const float MinDist = 200.0f;

    for (int32 i = 0; i < HerdMembers.Num(); ++i)
    {
        if (i == MemberIndex) continue;
        FVector Diff = Self.Location - HerdMembers[i].Location;
        float Dist = Diff.Size();
        if (Dist < MinDist && Dist > 0.0f)
        {
            SeparationVec += Diff.GetSafeNormal() * (MinDist - Dist);
        }
    }
    return SeparationVec;
}

FVector ACrowd_StampedeController::CalculateCohesion(int32 MemberIndex) const
{
    const FCrowd_HerdMember& Self = HerdMembers[MemberIndex];
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;
    const float NeighborRadius = 800.0f;

    for (int32 i = 0; i < HerdMembers.Num(); ++i)
    {
        if (i == MemberIndex) continue;
        float Dist = FVector::Dist(Self.Location, HerdMembers[i].Location);
        if (Dist < NeighborRadius)
        {
            CenterOfMass += HerdMembers[i].Location;
            ++Count;
        }
    }

    if (Count > 0)
    {
        CenterOfMass /= static_cast<float>(Count);
        return (CenterOfMass - Self.Location).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

FVector ACrowd_StampedeController::CalculateAlignment(int32 MemberIndex) const
{
    const FCrowd_HerdMember& Self = HerdMembers[MemberIndex];
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;
    const float NeighborRadius = 600.0f;

    for (int32 i = 0; i < HerdMembers.Num(); ++i)
    {
        if (i == MemberIndex) continue;
        float Dist = FVector::Dist(Self.Location, HerdMembers[i].Location);
        if (Dist < NeighborRadius)
        {
            AvgVelocity += HerdMembers[i].Velocity;
            ++Count;
        }
    }

    if (Count > 0)
    {
        AvgVelocity /= static_cast<float>(Count);
        return AvgVelocity.GetSafeNormal();
    }
    return FVector::ZeroVector;
}

float ACrowd_StampedeController::AverageFearLevel() const
{
    if (HerdMembers.Num() == 0) return 0.0f;
    float Total = 0.0f;
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        Total += Member.FearLevel;
    }
    return Total / static_cast<float>(HerdMembers.Num());
}
