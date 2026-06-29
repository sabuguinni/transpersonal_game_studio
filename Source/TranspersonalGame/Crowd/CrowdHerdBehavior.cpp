#include "CrowdHerdBehavior.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCrowdHerdBehavior::UCrowdHerdBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20 Hz tick for performance
}

void UCrowdHerdBehavior::BeginPlay()
{
    Super::BeginPlay();
    CurrentHerdState = ECrowd_HerdState::Grazing;
    StateTimer = 0.0f;
}

void UCrowdHerdBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Decay fear across all members
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        Member.FearLevel = FMath::Max(0.0f, Member.FearLevel - FearDecayRate * DeltaTime);
    }

    StateTimer += DeltaTime;

    switch (CurrentHerdState)
    {
        case ECrowd_HerdState::Grazing:    UpdateGrazing(DeltaTime);    break;
        case ECrowd_HerdState::Wandering:  UpdateWandering(DeltaTime);  break;
        case ECrowd_HerdState::Fleeing:    UpdateFleeing(DeltaTime);    break;
        case ECrowd_HerdState::Stampeding: UpdateStampeding(DeltaTime); break;
        case ECrowd_HerdState::Resting:    UpdateResting(DeltaTime);    break;
    }
}

void UCrowdHerdBehavior::AddHerdMember(AActor* Member)
{
    if (!Member || HerdMembers.Num() >= MaxHerdSize) return;

    FCrowd_HerdMember NewMember;
    NewMember.MemberActor = Member;
    NewMember.Velocity = FVector::ZeroVector;
    NewMember.FearLevel = 0.0f;
    NewMember.bIsLeader = (HerdMembers.Num() == 0); // First member is leader
    HerdMembers.Add(NewMember);
}

void UCrowdHerdBehavior::RemoveHerdMember(AActor* Member)
{
    HerdMembers.RemoveAll([Member](const FCrowd_HerdMember& M) {
        return M.MemberActor == Member;
    });

    // Reassign leader if needed
    if (HerdMembers.Num() > 0)
    {
        bool bHasLeader = false;
        for (const FCrowd_HerdMember& M : HerdMembers)
        {
            if (M.bIsLeader) { bHasLeader = true; break; }
        }
        if (!bHasLeader) HerdMembers[0].bIsLeader = true;
    }
}

void UCrowdHerdBehavior::TriggerStampede(FVector ThreatLocation)
{
    StampedeThreatLocation = ThreatLocation;
    SetHerdState(ECrowd_HerdState::Stampeding);

    // Spike fear on all members
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        Member.FearLevel = 1.0f;
    }
}

void UCrowdHerdBehavior::SetHerdState(ECrowd_HerdState NewState)
{
    CurrentHerdState = NewState;
    StateTimer = 0.0f;
}

FVector UCrowdHerdBehavior::ComputeFlockingVelocity(const FCrowd_HerdMember& Member)
{
    FVector Sep = ComputeSeparation(Member);
    FVector Coh = ComputeCohesion(Member);
    FVector Ali = ComputeAlignment(Member);

    // Weight the three forces
    return (Sep * 1.5f) + (Coh * 0.8f) + (Ali * 1.0f);
}

FVector UCrowdHerdBehavior::ComputeSeparation(const FCrowd_HerdMember& Member)
{
    if (!Member.MemberActor) return FVector::ZeroVector;

    FVector Steer = FVector::ZeroVector;
    int32 Count = 0;
    FVector MyPos = Member.MemberActor->GetActorLocation();

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (!Other.MemberActor || Other.MemberActor == Member.MemberActor) continue;
        float Dist = FVector::Dist(MyPos, Other.MemberActor->GetActorLocation());
        if (Dist < SeparationRadius && Dist > 0.0f)
        {
            FVector Diff = (MyPos - Other.MemberActor->GetActorLocation()).GetSafeNormal() / Dist;
            Steer += Diff;
            Count++;
        }
    }

    if (Count > 0) Steer /= (float)Count;
    return Steer;
}

FVector UCrowdHerdBehavior::ComputeCohesion(const FCrowd_HerdMember& Member)
{
    if (!Member.MemberActor) return FVector::ZeroVector;

    FVector Center = FVector::ZeroVector;
    int32 Count = 0;
    FVector MyPos = Member.MemberActor->GetActorLocation();

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (!Other.MemberActor || Other.MemberActor == Member.MemberActor) continue;
        float Dist = FVector::Dist(MyPos, Other.MemberActor->GetActorLocation());
        if (Dist < CohesionRadius)
        {
            Center += Other.MemberActor->GetActorLocation();
            Count++;
        }
    }

    if (Count > 0)
    {
        Center /= (float)Count;
        return (Center - MyPos).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

FVector UCrowdHerdBehavior::ComputeAlignment(const FCrowd_HerdMember& Member)
{
    if (!Member.MemberActor) return FVector::ZeroVector;

    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;
    FVector MyPos = Member.MemberActor->GetActorLocation();

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (!Other.MemberActor || Other.MemberActor == Member.MemberActor) continue;
        float Dist = FVector::Dist(MyPos, Other.MemberActor->GetActorLocation());
        if (Dist < AlignmentRadius)
        {
            AvgVelocity += Other.Velocity;
            Count++;
        }
    }

    if (Count > 0)
    {
        AvgVelocity /= (float)Count;
        return AvgVelocity.GetSafeNormal();
    }
    return FVector::ZeroVector;
}

void UCrowdHerdBehavior::UpdateGrazing(float DeltaTime)
{
    // Occasionally transition to wandering
    if (StateTimer > 30.0f)
    {
        if (FMath::RandBool())
        {
            SetHerdState(ECrowd_HerdState::Wandering);
        }
        else
        {
            SetHerdState(ECrowd_HerdState::Resting);
        }
    }

    // Slow drift with separation only
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor) continue;
        FVector Sep = ComputeSeparation(Member);
        Member.Velocity = FMath::VInterpTo(Member.Velocity, Sep * GrazeSpeed, DeltaTime, 2.0f);
        FVector NewLoc = Member.MemberActor->GetActorLocation() + Member.Velocity * DeltaTime;
        Member.MemberActor->SetActorLocation(NewLoc, true);
    }
}

void UCrowdHerdBehavior::UpdateWandering(float DeltaTime)
{
    if (StateTimer > 20.0f)
    {
        SetHerdState(ECrowd_HerdState::Grazing);
    }

    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor) continue;
        FVector Flock = ComputeFlockingVelocity(Member);
        Member.Velocity = FMath::VInterpTo(Member.Velocity, Flock * WanderSpeed, DeltaTime, 1.5f);
        FVector NewLoc = Member.MemberActor->GetActorLocation() + Member.Velocity * DeltaTime;
        Member.MemberActor->SetActorLocation(NewLoc, true);
    }
}

void UCrowdHerdBehavior::UpdateFleeing(float DeltaTime)
{
    // Check if fear has subsided
    float AvgFear = 0.0f;
    for (const FCrowd_HerdMember& M : HerdMembers) AvgFear += M.FearLevel;
    if (HerdMembers.Num() > 0) AvgFear /= HerdMembers.Num();

    if (AvgFear < 0.1f)
    {
        SetHerdState(ECrowd_HerdState::Grazing);
        return;
    }

    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor) continue;
        FVector AwayFromThreat = (Member.MemberActor->GetActorLocation() - StampedeThreatLocation).GetSafeNormal();
        FVector Sep = ComputeSeparation(Member);
        FVector FleeDir = (AwayFromThreat * 2.0f + Sep).GetSafeNormal();
        Member.Velocity = FMath::VInterpTo(Member.Velocity, FleeDir * StampedeSpeed * 0.6f, DeltaTime, 3.0f);
        FVector NewLoc = Member.MemberActor->GetActorLocation() + Member.Velocity * DeltaTime;
        Member.MemberActor->SetActorLocation(NewLoc, true);
    }
}

void UCrowdHerdBehavior::UpdateStampeding(float DeltaTime)
{
    if (StateTimer > 15.0f)
    {
        SetHerdState(ECrowd_HerdState::Fleeing);
        return;
    }

    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor) continue;
        FVector AwayFromThreat = (Member.MemberActor->GetActorLocation() - StampedeThreatLocation).GetSafeNormal();
        FVector Sep = ComputeSeparation(Member);
        FVector Ali = ComputeAlignment(Member);
        FVector StampedeDir = (AwayFromThreat * 3.0f + Sep * 1.5f + Ali).GetSafeNormal();
        Member.Velocity = FMath::VInterpTo(Member.Velocity, StampedeDir * StampedeSpeed, DeltaTime, 5.0f);
        FVector NewLoc = Member.MemberActor->GetActorLocation() + Member.Velocity * DeltaTime;
        Member.MemberActor->SetActorLocation(NewLoc, true);
    }
}

void UCrowdHerdBehavior::UpdateResting(float DeltaTime)
{
    if (StateTimer > 45.0f)
    {
        SetHerdState(ECrowd_HerdState::Grazing);
    }

    // Slow drift to tight cluster
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor) continue;
        FVector Sep = ComputeSeparation(Member);
        Member.Velocity = FMath::VInterpTo(Member.Velocity, Sep * 20.0f, DeltaTime, 1.0f);
        FVector NewLoc = Member.MemberActor->GetActorLocation() + Member.Velocity * DeltaTime;
        Member.MemberActor->SetActorLocation(NewLoc, true);
    }
}
