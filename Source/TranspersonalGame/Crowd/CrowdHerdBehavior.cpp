#include "CrowdHerdBehavior.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// UCrowd_HerdBehaviorComponent
// ============================================================

UCrowd_HerdBehaviorComponent::UCrowd_HerdBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    HerbivoreSpecies = ECrowd_HerbivoreType::Triceratops;
    CurrentState = ECrowd_HerdBehaviorState::Grazing;
    HerdSize = 0;
    HerdCentroid = FVector::ZeroVector;
    CollectiveFearLevel = 0.f;
    FearDecayRate = 0.05f;
    StampedeTriggerFear = 0.75f;
    StampedeTimer = 0.f;
    bStampedeActive = false;
    StampedeDirection = FVector::ForwardVector;
}

void UCrowd_HerdBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCrowd_HerdBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (HerdMembers.Num() == 0) return;

    UpdateFearDecay(DeltaTime);
    UpdateHerdCentroid();
    UpdateBoidsBehavior(DeltaTime);

    // Stampede timer
    if (bStampedeActive)
    {
        StampedeTimer -= DeltaTime;
        if (StampedeTimer <= 0.f)
        {
            bStampedeActive = false;
            CurrentState = ECrowd_HerdBehaviorState::Alerted;
            unreal_log("Herd stampede ended — transitioning to Alerted");
        }
    }
}

void UCrowd_HerdBehaviorComponent::InitializeHerd(int32 NumMembers, FVector SpawnCenter)
{
    HerdMembers.Empty();
    int32 ClampedSize = FMath::Clamp(NumMembers, HerdConfig.MinHerdSize, HerdConfig.MaxHerdSize);
    HerdSize = ClampedSize;

    for (int32 i = 0; i < ClampedSize; i++)
    {
        FCrowd_HerdMemberData Member;
        float Angle = (float)i / (float)ClampedSize * 2.f * PI;
        float Radius = FMath::RandRange(100.f, HerdConfig.SeparationDistance * 2.f);
        Member.Location = SpawnCenter + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
        Member.Velocity = FVector::ZeroVector;
        Member.Health = 100.f;
        Member.bIsAlpha = (i == 0); // First member is alpha
        Member.Species = HerbivoreSpecies;
        Member.FearLevel = 0.f;
        HerdMembers.Add(Member);
    }

    UpdateHerdCentroid();
    CurrentState = ECrowd_HerdBehaviorState::Grazing;
}

void UCrowd_HerdBehaviorComponent::TriggerAlert(FVector ThreatLocation, float ThreatIntensity)
{
    CollectiveFearLevel = FMath::Clamp(CollectiveFearLevel + ThreatIntensity, 0.f, 1.f);

    // Propagate fear to individual members based on proximity to threat
    for (FCrowd_HerdMemberData& Member : HerdMembers)
    {
        float DistToThreat = FVector::Dist(Member.Location, ThreatLocation);
        float ProximityFear = FMath::Clamp(1.f - (DistToThreat / HerdConfig.AlertRadius), 0.f, 1.f);
        Member.FearLevel = FMath::Clamp(Member.FearLevel + ProximityFear * ThreatIntensity, 0.f, 1.f);
    }

    if (CurrentState == ECrowd_HerdBehaviorState::Grazing || CurrentState == ECrowd_HerdBehaviorState::Resting)
    {
        CurrentState = ECrowd_HerdBehaviorState::Alerted;
    }

    // Trigger stampede if fear exceeds threshold
    if (CollectiveFearLevel >= StampedeTriggerFear)
    {
        FVector FleeDir = (HerdCentroid - ThreatLocation).GetSafeNormal();
        TriggerStampede(FleeDir);
    }
}

void UCrowd_HerdBehaviorComponent::TriggerStampede(FVector FleeDirection)
{
    CurrentState = ECrowd_HerdBehaviorState::Stampeding;
    StampedeDirection = FleeDirection.GetSafeNormal();
    StampedeTimer = 15.f; // Stampede lasts 15 seconds
    bStampedeActive = true;

    // Give all members high velocity in flee direction
    for (FCrowd_HerdMemberData& Member : HerdMembers)
    {
        float SpeedVariance = FMath::RandRange(0.8f, 1.2f);
        Member.Velocity = StampedeDirection * HerdConfig.StampedeSpeed * SpeedVariance;
        Member.FearLevel = 1.f;
    }
}

void UCrowd_HerdBehaviorComponent::CalmHerd()
{
    CollectiveFearLevel = 0.f;
    bStampedeActive = false;
    StampedeTimer = 0.f;
    CurrentState = ECrowd_HerdBehaviorState::Grazing;

    for (FCrowd_HerdMemberData& Member : HerdMembers)
    {
        Member.FearLevel = 0.f;
        Member.Velocity = FVector::ZeroVector;
    }
}

FVector UCrowd_HerdBehaviorComponent::GetHerdCentroid() const
{
    return HerdCentroid;
}

bool UCrowd_HerdBehaviorComponent::IsStampeding() const
{
    return bStampedeActive;
}

float UCrowd_HerdBehaviorComponent::GetCollectiveFear() const
{
    return CollectiveFearLevel;
}

int32 UCrowd_HerdBehaviorComponent::GetHerdSize() const
{
    return HerdMembers.Num();
}

void UCrowd_HerdBehaviorComponent::UpdateBoidsBehavior(float DeltaTime)
{
    if (CurrentState == ECrowd_HerdBehaviorState::Stampeding)
    {
        // During stampede, apply strong flee force + separation
        for (int32 i = 0; i < HerdMembers.Num(); i++)
        {
            FVector StampedeForce = ComputeStampedeForce(i);
            FVector Separation = ComputeSeparation(i) * 2.f; // Extra separation during stampede
            HerdMembers[i].Velocity = (StampedeForce + Separation).GetClampedToMaxSize(HerdConfig.StampedeSpeed);
            HerdMembers[i].Location += HerdMembers[i].Velocity * DeltaTime;
        }
        return;
    }

    if (CurrentState == ECrowd_HerdBehaviorState::Grazing || CurrentState == ECrowd_HerdBehaviorState::Wandering)
    {
        // Normal boids: separation + alignment + cohesion
        for (int32 i = 0; i < HerdMembers.Num(); i++)
        {
            FVector Sep = ComputeSeparation(i) * 1.5f;
            FVector Ali = ComputeAlignment(i) * 1.0f;
            FVector Coh = ComputeCohesion(i) * HerdConfig.CohesionStrength;

            FVector Acceleration = Sep + Ali + Coh;
            float WanderSpeed = 150.f;
            HerdMembers[i].Velocity = (HerdMembers[i].Velocity + Acceleration * DeltaTime).GetClampedToMaxSize(WanderSpeed);
            HerdMembers[i].Location += HerdMembers[i].Velocity * DeltaTime;
        }
    }
}

void UCrowd_HerdBehaviorComponent::UpdateHerdCentroid()
{
    if (HerdMembers.Num() == 0) return;

    FVector Sum = FVector::ZeroVector;
    for (const FCrowd_HerdMemberData& Member : HerdMembers)
    {
        Sum += Member.Location;
    }
    HerdCentroid = Sum / (float)HerdMembers.Num();
}

void UCrowd_HerdBehaviorComponent::UpdateFearDecay(float DeltaTime)
{
    CollectiveFearLevel = FMath::Max(0.f, CollectiveFearLevel - FearDecayRate * DeltaTime);

    for (FCrowd_HerdMemberData& Member : HerdMembers)
    {
        Member.FearLevel = FMath::Max(0.f, Member.FearLevel - FearDecayRate * DeltaTime);
    }

    // Transition back to calm states
    if (CollectiveFearLevel < 0.1f && CurrentState == ECrowd_HerdBehaviorState::Alerted)
    {
        CurrentState = ECrowd_HerdBehaviorState::Grazing;
    }
}

FVector UCrowd_HerdBehaviorComponent::ComputeSeparation(int32 MemberIndex) const
{
    FVector Force = FVector::ZeroVector;
    const FCrowd_HerdMemberData& Self = HerdMembers[MemberIndex];

    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        if (i == MemberIndex) continue;
        float Dist = FVector::Dist(Self.Location, HerdMembers[i].Location);
        if (Dist < HerdConfig.SeparationDistance && Dist > 0.f)
        {
            FVector Away = (Self.Location - HerdMembers[i].Location).GetSafeNormal();
            Force += Away * (HerdConfig.SeparationDistance - Dist) / HerdConfig.SeparationDistance;
        }
    }
    return Force;
}

FVector UCrowd_HerdBehaviorComponent::ComputeAlignment(int32 MemberIndex) const
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;
    const FCrowd_HerdMemberData& Self = HerdMembers[MemberIndex];

    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        if (i == MemberIndex) continue;
        float Dist = FVector::Dist(Self.Location, HerdMembers[i].Location);
        if (Dist < HerdConfig.AlertRadius)
        {
            AvgVelocity += HerdMembers[i].Velocity;
            Count++;
        }
    }

    if (Count > 0)
    {
        AvgVelocity /= (float)Count;
        return (AvgVelocity - Self.Velocity).GetSafeNormal();
    }
    return FVector::ZeroVector;
}

FVector UCrowd_HerdBehaviorComponent::ComputeCohesion(int32 MemberIndex) const
{
    const FCrowd_HerdMemberData& Self = HerdMembers[MemberIndex];
    return (HerdCentroid - Self.Location).GetSafeNormal();
}

FVector UCrowd_HerdBehaviorComponent::ComputeStampedeForce(int32 MemberIndex) const
{
    const FCrowd_HerdMemberData& Self = HerdMembers[MemberIndex];
    // Stampede: flee direction + slight randomness for realism
    FVector RandomOffset = FVector(FMath::RandRange(-0.2f, 0.2f), FMath::RandRange(-0.2f, 0.2f), 0.f);
    return (StampedeDirection + RandomOffset).GetSafeNormal() * HerdConfig.StampedeSpeed;
}

// ============================================================
// ACrowd_HerdActor
// ============================================================

ACrowd_HerdActor::ACrowd_HerdActor()
{
    PrimaryActorTick.bCanEverTick = true;

    HerdBehavior = CreateDefaultSubobject<UCrowd_HerdBehaviorComponent>(TEXT("HerdBehavior"));
    DefaultSpecies = ECrowd_HerbivoreType::Triceratops;
    InitialHerdSize = 12;
}

void ACrowd_HerdActor::BeginPlay()
{
    Super::BeginPlay();
    SpawnHerd();
}

void ACrowd_HerdActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACrowd_HerdActor::SpawnHerd()
{
    if (HerdBehavior)
    {
        HerdBehavior->HerbivoreSpecies = DefaultSpecies;
        HerdBehavior->InitializeHerd(InitialHerdSize, GetActorLocation());
    }
}

void ACrowd_HerdActor::OnPredatorDetected(FVector PredatorLocation)
{
    if (HerdBehavior)
    {
        float ThreatIntensity = 0.8f;
        HerdBehavior->TriggerAlert(PredatorLocation, ThreatIntensity);
    }
}
