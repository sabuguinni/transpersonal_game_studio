// CrowdHerdBehavior.cpp
// Agent #13 — Crowd & Traffic Simulation
// Full implementation of prehistoric herd flocking, migration, threat response, and LOD

#include "CrowdHerdBehavior.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// Constructor
// ============================================================

UCrowd_HerdBehaviorComponent::UCrowd_HerdBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz tick for performance
}

// ============================================================
// BeginPlay
// ============================================================

void UCrowd_HerdBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Auto-elect leader if we have members
    if (HerdMembers.Num() > 0)
    {
        ElectNewLeader();
    }

    // Set default migration waypoints if none provided
    if (bMigrationEnabled && MigrationWaypoints.Num() == 0)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            FVector Origin = Owner->GetActorLocation();
            MigrationWaypoints.Add(Origin + FVector(3000.0f, 0.0f, 0.0f));
            MigrationWaypoints.Add(Origin + FVector(3000.0f, 3000.0f, 0.0f));
            MigrationWaypoints.Add(Origin + FVector(0.0f, 3000.0f, 0.0f));
            MigrationWaypoints.Add(Origin);
        }
    }
}

// ============================================================
// TickComponent
// ============================================================

void UCrowd_HerdBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (HerdMembers.Num() == 0) return;

    // Update centroid and distances
    UpdateHerdCentroid();
    UpdateMemberDistances();

    // Periodic threat scan
    TimeSinceLastScan += DeltaTime;
    if (TimeSinceLastScan >= ScanInterval)
    {
        ScanForThreats();
        TimeSinceLastScan = 0.0f;
    }

    // Decay panic
    DecayPanicLevels(DeltaTime);

    // State machine
    switch (CurrentState)
    {
        case ECrowd_HerdState::Grazing:
        case ECrowd_HerdState::Wandering:
        case ECrowd_HerdState::Drinking:
        case ECrowd_HerdState::Resting:
            ProcessGrazingState(DeltaTime);
            break;

        case ECrowd_HerdState::Alert:
            ProcessAlertState(DeltaTime);
            break;

        case ECrowd_HerdState::Fleeing:
            ProcessFleeingState(DeltaTime);
            break;

        case ECrowd_HerdState::Stampeding:
            ProcessStampedeState(DeltaTime);
            break;

        case ECrowd_HerdState::Regrouping:
            ProcessGrazingState(DeltaTime); // Regroup uses gentle cohesion
            break;
    }

    // Migration check
    if (bMigrationEnabled && MigrationWaypoints.Num() > 0)
    {
        TimeSinceLastMigrationCheck += DeltaTime;
        if (TimeSinceLastMigrationCheck >= MigrationCheckInterval)
        {
            ProcessMigration(DeltaTime);
            TimeSinceLastMigrationCheck = 0.0f;
        }
    }
}

// ============================================================
// Public Methods
// ============================================================

void UCrowd_HerdBehaviorComponent::RegisterHerdMember(AActor* MemberActor, ECrowd_HerdRole Role)
{
    if (!MemberActor) return;

    // Check if already registered
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberActor == MemberActor) return;
    }

    FCrowd_HerdMember NewMember;
    NewMember.MemberActor = MemberActor;
    NewMember.Role = Role;
    NewMember.bIsLeader = (Role == ECrowd_HerdRole::Leader);
    NewMember.TargetLocation = MemberActor->GetActorLocation();

    HerdMembers.Add(NewMember);

    if (Role == ECrowd_HerdRole::Leader)
    {
        HerdLeader = MemberActor;
    }

    // Auto-elect leader if none exists
    if (!HerdLeader && HerdMembers.Num() > 0)
    {
        ElectNewLeader();
    }
}

void UCrowd_HerdBehaviorComponent::RemoveHerdMember(AActor* MemberActor)
{
    HerdMembers.RemoveAll([MemberActor](const FCrowd_HerdMember& M)
    {
        return M.MemberActor == MemberActor;
    });

    if (HerdLeader == MemberActor)
    {
        HerdLeader = nullptr;
        ElectNewLeader();
    }
}

void UCrowd_HerdBehaviorComponent::SetHerdState(ECrowd_HerdState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
}

void UCrowd_HerdBehaviorComponent::TriggerFleeResponse(AActor* ThreatActor, float ThreatIntensity)
{
    if (!ThreatActor) return;

    CurrentThreat = ThreatActor;
    ThreatDistance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());

    // Spread panic to all members
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        Member.PanicLevel = FMath::Clamp(Member.PanicLevel + ThreatIntensity, 0.0f, 1.0f);
    }

    float AvgPanic = GetAveragePanicLevel();
    if (AvgPanic >= HerdConfig.PanicThreshold)
    {
        SetHerdState(ECrowd_HerdState::Stampeding);
    }
    else
    {
        SetHerdState(ECrowd_HerdState::Fleeing);
    }
}

void UCrowd_HerdBehaviorComponent::TriggerStampede(FVector StampedeDirection)
{
    SetHerdState(ECrowd_HerdState::Stampeding);

    FVector NormalizedDir = StampedeDirection.GetSafeNormal();
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        Member.PanicLevel = 1.0f;
        Member.CurrentVelocity = NormalizedDir * HerdConfig.RunSpeed;
        if (Member.MemberActor)
        {
            Member.TargetLocation = Member.MemberActor->GetActorLocation() + NormalizedDir * 5000.0f;
        }
    }
}

void UCrowd_HerdBehaviorComponent::SetMigrationWaypoints(const TArray<FVector>& Waypoints)
{
    MigrationWaypoints = Waypoints;
    CurrentWaypointIndex = 0;
}

FVector UCrowd_HerdBehaviorComponent::ComputeFlockingForce(const FCrowd_HerdMember& Member) const
{
    FVector Alignment = ComputeAlignmentForce(Member) * HerdConfig.AlignmentWeight;
    FVector Cohesion = ComputeCohesionForce(Member) * HerdConfig.CohesionWeight;
    FVector Separation = ComputeSeparationForce(Member) * HerdConfig.SeparationWeight;

    return Alignment + Cohesion + Separation;
}

FVector UCrowd_HerdBehaviorComponent::GetHerdCentroid() const
{
    return HerdCentroid;
}

int32 UCrowd_HerdBehaviorComponent::GetHerdSize() const
{
    return HerdMembers.Num();
}

float UCrowd_HerdBehaviorComponent::GetAveragePanicLevel() const
{
    if (HerdMembers.Num() == 0) return 0.0f;

    float Total = 0.0f;
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        Total += Member.PanicLevel;
    }
    return Total / HerdMembers.Num();
}

void UCrowd_HerdBehaviorComponent::ElectNewLeader()
{
    if (HerdMembers.Num() == 0) return;

    // Prefer existing Elder or Scout roles, otherwise pick first
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.Role == ECrowd_HerdRole::Elder || Member.Role == ECrowd_HerdRole::Scout)
        {
            Member.bIsLeader = true;
            Member.Role = ECrowd_HerdRole::Leader;
            HerdLeader = Member.MemberActor;
            return;
        }
    }

    // Fallback: elect first member
    HerdMembers[0].bIsLeader = true;
    HerdMembers[0].Role = ECrowd_HerdRole::Leader;
    HerdLeader = HerdMembers[0].MemberActor;
}

void UCrowd_HerdBehaviorComponent::UpdateLODLevel(float DistanceToPlayer)
{
    if (DistanceToPlayer <= LOD0_Distance)
    {
        CurrentLODLevel = 0;
        PrimaryComponentTick.TickInterval = 0.05f; // 20Hz full simulation
    }
    else if (DistanceToPlayer <= LOD1_Distance)
    {
        CurrentLODLevel = 1;
        PrimaryComponentTick.TickInterval = 0.1f; // 10Hz medium
    }
    else if (DistanceToPlayer <= LOD2_Distance)
    {
        CurrentLODLevel = 2;
        PrimaryComponentTick.TickInterval = 0.25f; // 4Hz low
    }
    else
    {
        CurrentLODLevel = 3;
        PrimaryComponentTick.TickInterval = 1.0f; // 1Hz background
    }
}

// ============================================================
// Private Methods
// ============================================================

void UCrowd_HerdBehaviorComponent::UpdateHerdCentroid()
{
    if (HerdMembers.Num() == 0) return;

    FVector Sum = FVector::ZeroVector;
    int32 ValidCount = 0;

    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberActor)
        {
            Sum += Member.MemberActor->GetActorLocation();
            ValidCount++;
        }
    }

    if (ValidCount > 0)
    {
        HerdCentroid = Sum / ValidCount;
    }
}

void UCrowd_HerdBehaviorComponent::UpdateMemberDistances()
{
    if (!HerdLeader) return;

    FVector LeaderLoc = HerdLeader->GetActorLocation();
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberActor)
        {
            Member.DistanceToLeader = FVector::Dist(Member.MemberActor->GetActorLocation(), LeaderLoc);
        }
    }
}

void UCrowd_HerdBehaviorComponent::ProcessGrazingState(float DeltaTime)
{
    // Gentle cohesion toward centroid — members drift slowly together
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor) continue;

        FVector ToCenter = HerdCentroid - Member.MemberActor->GetActorLocation();
        float Dist = ToCenter.Size();

        if (Dist > HerdConfig.CohesionRadius)
        {
            // Drift back toward herd
            FVector MoveDir = ToCenter.GetSafeNormal();
            FVector NewLoc = Member.MemberActor->GetActorLocation() + MoveDir * HerdConfig.WalkSpeed * DeltaTime;
            Member.MemberActor->SetActorLocation(NewLoc, true);
            Member.CurrentVelocity = MoveDir * HerdConfig.WalkSpeed;
        }
        else
        {
            // Idle drift
            Member.CurrentVelocity = FVector::ZeroVector;
        }
    }
}

void UCrowd_HerdBehaviorComponent::ProcessAlertState(float DeltaTime)
{
    // Herd clusters together, faces threat direction
    if (!CurrentThreat) 
    {
        SetHerdState(ECrowd_HerdState::Grazing);
        return;
    }

    FVector ThreatDir = (CurrentThreat->GetActorLocation() - HerdCentroid).GetSafeNormal();

    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor) continue;

        // Move toward centroid for safety in numbers
        FVector ToCenter = HerdCentroid - Member.MemberActor->GetActorLocation();
        if (ToCenter.Size() > HerdConfig.SeparationRadius * 2.0f)
        {
            FVector NewLoc = Member.MemberActor->GetActorLocation() + ToCenter.GetSafeNormal() * HerdConfig.WalkSpeed * 0.5f * DeltaTime;
            Member.MemberActor->SetActorLocation(NewLoc, true);
        }

        // Face threat
        FRotator FaceRot = ThreatDir.Rotation();
        Member.MemberActor->SetActorRotation(FaceRot);
    }
}

void UCrowd_HerdBehaviorComponent::ProcessFleeingState(float DeltaTime)
{
    if (!CurrentThreat)
    {
        SetHerdState(ECrowd_HerdState::Regrouping);
        return;
    }

    FVector ThreatLoc = CurrentThreat->GetActorLocation();

    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor) continue;

        // Flee away from threat
        FVector AwayFromThreat = (Member.MemberActor->GetActorLocation() - ThreatLoc).GetSafeNormal();

        // Add flocking forces to stay with herd
        FVector FlockForce = ComputeFlockingForce(Member);
        FVector MoveDir = (AwayFromThreat * 0.7f + FlockForce.GetSafeNormal() * 0.3f).GetSafeNormal();

        float Speed = HerdConfig.RunSpeed * (0.8f + Member.PanicLevel * 0.2f);
        FVector NewLoc = Member.MemberActor->GetActorLocation() + MoveDir * Speed * DeltaTime;
        Member.MemberActor->SetActorLocation(NewLoc, true);
        Member.CurrentVelocity = MoveDir * Speed;

        // Face movement direction
        if (!MoveDir.IsNearlyZero())
        {
            Member.MemberActor->SetActorRotation(MoveDir.Rotation());
        }
    }
}

void UCrowd_HerdBehaviorComponent::ProcessStampedeState(float DeltaTime)
{
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor) continue;

        FVector ToTarget = (Member.TargetLocation - Member.MemberActor->GetActorLocation());
        float DistToTarget = ToTarget.Size();

        if (DistToTarget < WaypointAcceptanceRadius)
        {
            // Stampede target reached — transition to fleeing
            SetHerdState(ECrowd_HerdState::Fleeing);
            return;
        }

        FVector MoveDir = ToTarget.GetSafeNormal();

        // Add separation to avoid trampling each other
        FVector SepForce = ComputeSeparationForce(Member);
        MoveDir = (MoveDir + SepForce.GetSafeNormal() * 0.2f).GetSafeNormal();

        FVector NewLoc = Member.MemberActor->GetActorLocation() + MoveDir * HerdConfig.RunSpeed * DeltaTime;
        Member.MemberActor->SetActorLocation(NewLoc, true);
        Member.CurrentVelocity = MoveDir * HerdConfig.RunSpeed;

        if (!MoveDir.IsNearlyZero())
        {
            Member.MemberActor->SetActorRotation(MoveDir.Rotation());
        }
    }
}

void UCrowd_HerdBehaviorComponent::ProcessMigration(float DeltaTime)
{
    if (MigrationWaypoints.Num() == 0) return;
    if (CurrentState == ECrowd_HerdState::Fleeing || CurrentState == ECrowd_HerdState::Stampeding) return;

    FVector CurrentWaypoint = MigrationWaypoints[CurrentWaypointIndex];
    float DistToWaypoint = FVector::Dist(HerdCentroid, CurrentWaypoint);

    if (DistToWaypoint < WaypointAcceptanceRadius)
    {
        // Advance to next waypoint
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % MigrationWaypoints.Num();
        SetHerdState(ECrowd_HerdState::Wandering);
    }
    else if (CurrentState == ECrowd_HerdState::Grazing || CurrentState == ECrowd_HerdState::Resting)
    {
        // Occasionally start wandering toward waypoint
        SetHerdState(ECrowd_HerdState::Wandering);

        FVector MigrateDir = (CurrentWaypoint - HerdCentroid).GetSafeNormal();
        for (FCrowd_HerdMember& Member : HerdMembers)
        {
            if (Member.MemberActor)
            {
                Member.TargetLocation = Member.MemberActor->GetActorLocation() + MigrateDir * 500.0f;
            }
        }
    }
}

void UCrowd_HerdBehaviorComponent::DecayPanicLevels(float DeltaTime)
{
    bool bAnyPanic = false;
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.PanicLevel > 0.0f)
        {
            Member.PanicLevel = FMath::Max(0.0f, Member.PanicLevel - PanicDecayRate * DeltaTime);
            bAnyPanic = true;
        }
    }

    // If panic fully decayed and we were fleeing, regroup
    if (!bAnyPanic && (CurrentState == ECrowd_HerdState::Fleeing || CurrentState == ECrowd_HerdState::Stampeding))
    {
        CurrentThreat = nullptr;
        SetHerdState(ECrowd_HerdState::Regrouping);
    }
}

void UCrowd_HerdBehaviorComponent::ScanForThreats()
{
    UWorld* World = GetWorld();
    if (!World) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Find player pawn as potential threat
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(HerdCentroid, PlayerPawn->GetActorLocation());

    if (DistToPlayer <= HerdConfig.ThreatDetectionRadius)
    {
        ThreatDistance = DistToPlayer;
        float ThreatIntensity = 1.0f - (DistToPlayer / HerdConfig.ThreatDetectionRadius);

        if (CurrentState == ECrowd_HerdState::Grazing || CurrentState == ECrowd_HerdState::Wandering)
        {
            if (ThreatIntensity > 0.3f)
            {
                TriggerFleeResponse(PlayerPawn, ThreatIntensity);
            }
            else
            {
                CurrentThreat = PlayerPawn;
                SetHerdState(ECrowd_HerdState::Alert);
            }
        }
    }
    else
    {
        // Player out of range — clear threat if it was the player
        if (CurrentThreat == PlayerPawn)
        {
            CurrentThreat = nullptr;
        }
    }
}

FVector UCrowd_HerdBehaviorComponent::ComputeAlignmentForce(const FCrowd_HerdMember& Member) const
{
    if (!Member.MemberActor) return FVector::ZeroVector;

    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (Other.MemberActor == Member.MemberActor) continue;
        if (!Other.MemberActor) continue;

        float Dist = FVector::Dist(Member.MemberActor->GetActorLocation(), Other.MemberActor->GetActorLocation());
        if (Dist < HerdConfig.CohesionRadius)
        {
            AvgVelocity += Other.CurrentVelocity;
            Count++;
        }
    }

    if (Count == 0) return FVector::ZeroVector;
    return (AvgVelocity / Count).GetSafeNormal();
}

FVector UCrowd_HerdBehaviorComponent::ComputeCohesionForce(const FCrowd_HerdMember& Member) const
{
    if (!Member.MemberActor) return FVector::ZeroVector;

    FVector ToCenter = HerdCentroid - Member.MemberActor->GetActorLocation();
    return ToCenter.GetSafeNormal();
}

FVector UCrowd_HerdBehaviorComponent::ComputeSeparationForce(const FCrowd_HerdMember& Member) const
{
    if (!Member.MemberActor) return FVector::ZeroVector;

    FVector SeparationForce = FVector::ZeroVector;

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (Other.MemberActor == Member.MemberActor) continue;
        if (!Other.MemberActor) continue;

        FVector Diff = Member.MemberActor->GetActorLocation() - Other.MemberActor->GetActorLocation();
        float Dist = Diff.Size();

        if (Dist < HerdConfig.SeparationRadius && Dist > 0.0f)
        {
            // Stronger repulsion when closer
            SeparationForce += Diff.GetSafeNormal() * (HerdConfig.SeparationRadius / Dist);
        }
    }

    return SeparationForce.GetSafeNormal();
}
