#include "DinosaurHerdManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ADinosaurHerdManager::ADinosaurHerdManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = BehaviorTickInterval;

    CurrentHerdState = ECrowd_HerdState::Grazing;
    HerdCenterOfMass = FVector::ZeroVector;
    MigrationTarget = FVector::ZeroVector;

    StampedeThreatRadius = 1500.f;
    StampedeDuration = 15.f;
    bStampedeActive = false;
    StampedeTimer = 0.f;
    BehaviorTickAccumulator = 0.f;

    CohesionWeight = 1.0f;
    SeparationWeight = 1.5f;
    AlignmentWeight = 0.8f;
    LeaderFollowWeight = 1.2f;
}

void ADinosaurHerdManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeHerd(0);
}

void ADinosaurHerdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateHerdBehavior(DeltaTime);
}

void ADinosaurHerdManager::InitializeHerd(int32 HerdSizeOverride)
{
    HerdMembers.Empty();

    int32 Size = (HerdSizeOverride > 0) ? HerdSizeOverride : HerdConfig.HerdSize;
    FVector Origin = GetActorLocation();

    for (int32 i = 0; i < Size; i++)
    {
        FCrowd_HerdMember Member;
        Member.MemberIndex = i;
        Member.bIsLeader = (i == 0);
        Member.Health = 100.f;

        // Scatter members in a circle around origin
        float Angle = (float)i / (float)Size * 2.f * PI;
        float Radius = FMath::RandRange(100.f, HerdConfig.CohesionRadius * 0.5f);
        Member.Location = Origin + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.f
        );
        Member.Velocity = FVector::ZeroVector;

        HerdMembers.Add(Member);
    }

    HerdCenterOfMass = GetHerdCenterOfMass();
    UE_LOG(LogTemp, Log, TEXT("DinosaurHerdManager: Initialized herd of %d %s"),
        Size,
        *UEnum::GetValueAsString(HerdConfig.Species));
}

void ADinosaurHerdManager::UpdateHerdBehavior(float DeltaTime)
{
    if (HerdMembers.Num() == 0) return;

    // Update stampede timer
    if (bStampedeActive)
    {
        StampedeTimer -= DeltaTime;
        if (StampedeTimer <= 0.f)
        {
            bStampedeActive = false;
            TransitionHerdState(ECrowd_HerdState::Grazing);
            UE_LOG(LogTemp, Log, TEXT("DinosaurHerdManager: Stampede ended — herd returning to grazing"));
        }
    }

    // Recalculate center of mass
    HerdCenterOfMass = GetHerdCenterOfMass();

    // Check for threats periodically
    CheckForThreats();

    // Apply boid rules to each member
    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        ApplyBoidRules(i, DeltaTime);
    }
}

void ADinosaurHerdManager::ApplyBoidRules(int32 MemberIdx, float DeltaTime)
{
    if (!HerdMembers.IsValidIndex(MemberIdx)) return;

    FCrowd_HerdMember& Member = HerdMembers[MemberIdx];
    FVector Cohesion = FVector::ZeroVector;
    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    int32 CohesionCount = 0;
    int32 AlignmentCount = 0;

    for (int32 j = 0; j < HerdMembers.Num(); j++)
    {
        if (j == MemberIdx) continue;

        const FCrowd_HerdMember& Other = HerdMembers[j];
        float Dist = FVector::Dist(Member.Location, Other.Location);

        // Separation — avoid crowding
        if (Dist < HerdConfig.SeparationRadius && Dist > 0.f)
        {
            FVector Away = (Member.Location - Other.Location).GetSafeNormal();
            Separation += Away * (HerdConfig.SeparationRadius / Dist);
        }

        // Cohesion — move toward center of local flock
        if (Dist < HerdConfig.CohesionRadius)
        {
            Cohesion += Other.Location;
            CohesionCount++;
        }

        // Alignment — match velocity of nearby members
        if (Dist < HerdConfig.AlignmentRadius)
        {
            Alignment += Other.Velocity;
            AlignmentCount++;
        }
    }

    // Normalize cohesion toward center
    if (CohesionCount > 0)
    {
        Cohesion /= (float)CohesionCount;
        Cohesion = (Cohesion - Member.Location).GetSafeNormal();
    }

    if (AlignmentCount > 0)
    {
        Alignment /= (float)AlignmentCount;
        Alignment = Alignment.GetSafeNormal();
    }

    // Leader follow — non-leaders track leader
    FVector LeaderForce = FVector::ZeroVector;
    int32 LeaderIdx = GetHerdLeaderIndex();
    if (!Member.bIsLeader && HerdMembers.IsValidIndex(LeaderIdx))
    {
        FVector ToLeader = (HerdMembers[LeaderIdx].Location - Member.Location);
        float LeaderDist = ToLeader.Size();
        if (LeaderDist > HerdConfig.SeparationRadius)
        {
            LeaderForce = ToLeader.GetSafeNormal();
        }
    }

    // Migration target force
    FVector MigrationForce = FVector::ZeroVector;
    if (CurrentHerdState == ECrowd_HerdState::Migrating || CurrentHerdState == ECrowd_HerdState::Fleeing || CurrentHerdState == ECrowd_HerdState::Stampeding)
    {
        if (!MigrationTarget.IsZero())
        {
            MigrationForce = (MigrationTarget - Member.Location).GetSafeNormal();
        }
    }

    // Combine forces
    FVector SteeringForce =
        Cohesion * CohesionWeight +
        Separation * SeparationWeight +
        Alignment * AlignmentWeight +
        LeaderForce * LeaderFollowWeight +
        MigrationForce * 2.0f;

    float Speed = (CurrentHerdState == ECrowd_HerdState::Stampeding || CurrentHerdState == ECrowd_HerdState::Fleeing)
        ? HerdConfig.StampedeSpeed
        : HerdConfig.MoveSpeed;

    // Apply velocity
    Member.Velocity = FMath::VInterpTo(Member.Velocity, SteeringForce * Speed, DeltaTime, 3.f);
    Member.Location += Member.Velocity * DeltaTime;
}

void ADinosaurHerdManager::TriggerStampede(FVector ThreatLocation)
{
    if (bStampedeActive) return;

    bStampedeActive = true;
    StampedeTimer = StampedeDuration;

    // Flee direction = away from threat
    FVector FleeDir = (HerdCenterOfMass - ThreatLocation).GetSafeNormal();
    MigrationTarget = HerdCenterOfMass + FleeDir * 10000.f;

    TransitionHerdState(ECrowd_HerdState::Stampeding);

    UE_LOG(LogTemp, Warning, TEXT("DinosaurHerdManager: STAMPEDE TRIGGERED! Threat at %s, fleeing toward %s"),
        *ThreatLocation.ToString(), *MigrationTarget.ToString());
}

void ADinosaurHerdManager::SetMigrationTarget(FVector NewTarget)
{
    MigrationTarget = NewTarget;
    if (CurrentHerdState == ECrowd_HerdState::Grazing || CurrentHerdState == ECrowd_HerdState::Resting)
    {
        TransitionHerdState(ECrowd_HerdState::Migrating);
    }
}

FVector ADinosaurHerdManager::GetHerdCenterOfMass() const
{
    if (HerdMembers.Num() == 0) return GetActorLocation();

    FVector Sum = FVector::ZeroVector;
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        Sum += Member.Location;
    }
    return Sum / (float)HerdMembers.Num();
}

int32 ADinosaurHerdManager::GetHerdLeaderIndex() const
{
    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        if (HerdMembers[i].bIsLeader) return i;
    }
    return 0;
}

void ADinosaurHerdManager::CheckForThreats()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Sphere overlap check for predator pawns near herd center
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bThreatFound = World->OverlapMultiByChannel(
        Overlaps,
        HerdCenterOfMass,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(HerdConfig.ThreatDetectionRadius),
        Params
    );

    if (bThreatFound && !bStampedeActive)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* OverlapActor = Overlap.GetActor();
            if (OverlapActor && OverlapActor->ActorHasTag(FName("Predator")))
            {
                TriggerStampede(OverlapActor->GetActorLocation());
                break;
            }
        }
    }
}

void ADinosaurHerdManager::TransitionHerdState(ECrowd_HerdState NewState)
{
    if (CurrentHerdState == NewState) return;

    ECrowd_HerdState OldState = CurrentHerdState;
    CurrentHerdState = NewState;

    UE_LOG(LogTemp, Log, TEXT("DinosaurHerdManager: State transition %s -> %s"),
        *UEnum::GetValueAsString(OldState),
        *UEnum::GetValueAsString(NewState));
}

FString ADinosaurHerdManager::GetHerdStateAsString() const
{
    return UEnum::GetValueAsString(CurrentHerdState);
}
