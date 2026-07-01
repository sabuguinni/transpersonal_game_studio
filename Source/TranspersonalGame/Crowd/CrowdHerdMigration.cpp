#include "Crowd/CrowdHerdMigration.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// ACrowdHerdMigration — Full Implementation
// Brachiosaurus herd migration with Reynolds flocking,
// T-Rex territory avoidance, and stampede dynamics.
// ============================================================

ACrowdHerdMigration::ACrowdHerdMigration()
{
    PrimaryActorTick.bCanEverTick = true;

    HerdSize = 12;
    HerdCohesionRadius = 1200.f;
    SeparationRadius = 300.f;
    MigrationSpeed = 350.f;
    FleeSpeed = 700.f;
    CurrentState = ECrowd_HerdState::Grazing;
    CurrentWaypointIndex = 0;
    bLoopMigration = true;
    TerritoryAvoidanceWeight = 2.5f;
    CurrentPanicLevel = 0.f;
    PanicDecayRate = 0.05f;
    StampedeThreshold = 0.7f;
    GrazingTimer = 0.f;
    bStampedeActive = false;
    CurrentFleeDirection = FVector::ForwardVector;
}

void ACrowdHerdMigration::BeginPlay()
{
    Super::BeginPlay();
    InitializeHerd(HerdSize);

    // Set up default migration path if none configured
    if (MigrationPath.Num() == 0)
    {
        FCrowd_MigrationWaypoint WpA, WpB, WpC, WpD;
        WpA.Location = GetActorLocation() + FVector(-3000.f, -2000.f, 0.f);
        WpA.GrazingDuration = 45.f;
        WpA.bIsWaterSource = false;

        WpB.Location = GetActorLocation() + FVector(-1000.f, 3000.f, 0.f);
        WpB.GrazingDuration = 90.f;
        WpB.bIsWaterSource = true;   // River bend

        WpC.Location = GetActorLocation() + FVector(2000.f, 1000.f, 0.f);
        WpC.GrazingDuration = 60.f;
        WpC.bIsWaterSource = false;

        WpD.Location = GetActorLocation() + FVector(3500.f, -1500.f, 0.f);
        WpD.GrazingDuration = 30.f;
        WpD.bIsWaterSource = false;

        MigrationPath.Add(WpA);
        MigrationPath.Add(WpB);
        MigrationPath.Add(WpC);
        MigrationPath.Add(WpD);
    }

    // Set up default T-Rex territory zone
    if (TerritoryZonesToAvoid.Num() == 0)
    {
        FCrowd_TerritoryZone TRexZone;
        TRexZone.Center = GetActorLocation() + FVector(1500.f, 500.f, 0.f);
        TRexZone.Radius = 2500.f;
        TRexZone.DangerLevel = 1.0f;
        TRexZone.OwnerSpecies = TEXT("TRex");
        TerritoryZonesToAvoid.Add(TRexZone);
    }
}

void ACrowdHerdMigration::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Decay panic over time
    if (CurrentPanicLevel > 0.f)
    {
        CurrentPanicLevel = FMath::Max(0.f, CurrentPanicLevel - PanicDecayRate * DeltaTime);
        if (CurrentPanicLevel < StampedeThreshold && bStampedeActive)
        {
            bStampedeActive = false;
        }
    }

    // State machine evaluation
    ECrowd_HerdState NewState = EvaluateStateTransition();
    if (NewState != CurrentState)
    {
        SetHerdState(NewState);
    }

    // Update herd movement
    UpdateHerdMovement(DeltaTime);

    // Grazing timer
    if (CurrentState == ECrowd_HerdState::Grazing && MigrationPath.Num() > 0)
    {
        GrazingTimer += DeltaTime;
        float GrazeDuration = MigrationPath[CurrentWaypointIndex].GrazingDuration;
        if (GrazingTimer >= GrazeDuration)
        {
            GrazingTimer = 0.f;
            SetHerdState(ECrowd_HerdState::Migrating);
        }
    }
}

void ACrowdHerdMigration::InitializeHerd(int32 NumMembers)
{
    HerdMembers.Empty();
    HerdMembers.Reserve(NumMembers);

    FVector BasePos = GetActorLocation();

    for (int32 i = 0; i < NumMembers; ++i)
    {
        FCrowd_HerdMember Member;
        Member.MemberIndex = i;
        Member.bIsAlive = true;
        Member.Health = 1000.f;

        // Assign roles
        if (i == 0)
        {
            Member.Role = ECrowd_HerdMemberRole::Matriarch;
            Member.Speed = MigrationSpeed * 1.1f;
        }
        else if (i < 3)
        {
            Member.Role = ECrowd_HerdMemberRole::Scout;
            Member.Speed = MigrationSpeed * 1.2f;
        }
        else if (i < NumMembers - 2)
        {
            Member.Role = ECrowd_HerdMemberRole::Juvenile;
            Member.Speed = MigrationSpeed * 0.9f;
        }
        else
        {
            Member.Role = ECrowd_HerdMemberRole::Rear;
            Member.Speed = MigrationSpeed;
        }

        // Scatter initial positions in a cluster
        float Angle = (float)i / (float)NumMembers * 2.f * PI;
        float Radius = FMath::RandRange(100.f, HerdCohesionRadius * 0.5f);
        Member.Position = BasePos + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.f
        );

        HerdMembers.Add(Member);
    }
}

void ACrowdHerdMigration::UpdateHerdMovement(float DeltaTime)
{
    if (HerdMembers.Num() == 0 || MigrationPath.Num() == 0) return;

    FVector TargetLocation = MigrationPath[CurrentWaypointIndex].Location;
    FVector Centroid = GetHerdCentroid();
    float CurrentSpeed = (CurrentState == ECrowd_HerdState::Fleeing || bStampedeActive) ? FleeSpeed : MigrationSpeed;

    for (int32 i = 0; i < HerdMembers.Num(); ++i)
    {
        if (!HerdMembers[i].bIsAlive) continue;

        FVector MoveDir = FVector::ZeroVector;

        if (CurrentState == ECrowd_HerdState::Migrating)
        {
            // Primary: move toward waypoint
            FVector ToWaypoint = (TargetLocation - HerdMembers[i].Position).GetSafeNormal();
            MoveDir += ToWaypoint * 1.5f;

            // Flocking forces
            MoveDir += CalculateSeparation(i) * 1.2f;
            MoveDir += CalculateCohesion(i) * 0.8f;
            MoveDir += CalculateAlignment(i) * 0.6f;

            // Territory avoidance
            MoveDir += CalculateAvoidanceVector(HerdMembers[i].Position) * TerritoryAvoidanceWeight;
        }
        else if (CurrentState == ECrowd_HerdState::Fleeing || bStampedeActive)
        {
            MoveDir = CurrentFleeDirection * 2.f;
            MoveDir += CalculateSeparation(i) * 0.5f;
        }
        else if (CurrentState == ECrowd_HerdState::Grazing)
        {
            // Slow random wander within cohesion radius
            FVector RandomDir = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 0.f).GetSafeNormal();
            MoveDir = RandomDir * 0.3f;
            MoveDir += CalculateSeparation(i) * 0.8f;
        }

        if (!MoveDir.IsNearlyZero())
        {
            MoveDir = MoveDir.GetSafeNormal();
            float Speed = HerdMembers[i].Speed;
            if (bStampedeActive) Speed = FleeSpeed * 1.3f;
            HerdMembers[i].Position += MoveDir * Speed * DeltaTime;
        }
    }

    // Check waypoint arrival (matriarch leads)
    FVector MatriarchPos = GetMatriarchPosition();
    float DistToWaypoint = FVector::Dist(MatriarchPos, TargetLocation);
    if (DistToWaypoint < MigrationPath[CurrentWaypointIndex].AcceptanceRadius)
    {
        AdvanceToNextWaypoint();
    }
}

void ACrowdHerdMigration::AdvanceToNextWaypoint()
{
    if (MigrationPath.Num() == 0) return;

    CurrentWaypointIndex++;
    if (CurrentWaypointIndex >= MigrationPath.Num())
    {
        if (bLoopMigration)
        {
            CurrentWaypointIndex = 0;
        }
        else
        {
            CurrentWaypointIndex = MigrationPath.Num() - 1;
            SetHerdState(ECrowd_HerdState::Grazing);
            return;
        }
    }

    // Arrive at water source → switch to Drinking
    if (MigrationPath[CurrentWaypointIndex].bIsWaterSource)
    {
        SetHerdState(ECrowd_HerdState::Drinking);
        GrazingTimer = 0.f;
    }
    else
    {
        SetHerdState(ECrowd_HerdState::Grazing);
        GrazingTimer = 0.f;
    }
}

FVector ACrowdHerdMigration::GetHerdCentroid() const
{
    FVector Sum = FVector::ZeroVector;
    int32 Count = 0;
    for (const FCrowd_HerdMember& M : HerdMembers)
    {
        if (M.bIsAlive)
        {
            Sum += M.Position;
            Count++;
        }
    }
    return Count > 0 ? Sum / (float)Count : GetActorLocation();
}

void ACrowdHerdMigration::ReactToThreat(FVector ThreatLocation, float ThreatRadius, float PanicAmount)
{
    FVector Centroid = GetHerdCentroid();
    float DistToThreat = FVector::Dist(Centroid, ThreatLocation);

    if (DistToThreat < ThreatRadius)
    {
        float Proximity = 1.f - (DistToThreat / ThreatRadius);
        CurrentPanicLevel = FMath::Min(1.f, CurrentPanicLevel + PanicAmount * Proximity);

        // Flee away from threat
        CurrentFleeDirection = (Centroid - ThreatLocation).GetSafeNormal();
        CurrentFleeDirection.Z = 0.f;

        if (CurrentPanicLevel >= StampedeThreshold)
        {
            TriggerStampede(CurrentFleeDirection);
        }
        else
        {
            SetHerdState(ECrowd_HerdState::Fleeing);
        }
    }
}

void ACrowdHerdMigration::TriggerStampede(FVector FleeDirection)
{
    bStampedeActive = true;
    CurrentFleeDirection = FleeDirection.GetSafeNormal();
    CurrentFleeDirection.Z = 0.f;
    SetHerdState(ECrowd_HerdState::Stampeding);
    CurrentPanicLevel = 1.0f;
}

bool ACrowdHerdMigration::IsInTerritoryZone(FVector Location) const
{
    for (const FCrowd_TerritoryZone& Zone : TerritoryZonesToAvoid)
    {
        if (FVector::Dist(Location, Zone.Center) < Zone.Radius)
        {
            return true;
        }
    }
    return false;
}

FVector ACrowdHerdMigration::CalculateSeparation(int32 MemberIndex) const
{
    if (!HerdMembers.IsValidIndex(MemberIndex)) return FVector::ZeroVector;

    FVector SteerAway = FVector::ZeroVector;
    const FVector& MyPos = HerdMembers[MemberIndex].Position;

    for (int32 j = 0; j < HerdMembers.Num(); ++j)
    {
        if (j == MemberIndex || !HerdMembers[j].bIsAlive) continue;
        float Dist = FVector::Dist(MyPos, HerdMembers[j].Position);
        if (Dist < SeparationRadius && Dist > 0.f)
        {
            FVector Away = (MyPos - HerdMembers[j].Position) / Dist;
            SteerAway += Away * (SeparationRadius / Dist);
        }
    }
    return SteerAway.GetSafeNormal();
}

FVector ACrowdHerdMigration::CalculateCohesion(int32 MemberIndex) const
{
    if (!HerdMembers.IsValidIndex(MemberIndex)) return FVector::ZeroVector;

    FVector Centroid = GetHerdCentroid();
    FVector ToCentroid = (Centroid - HerdMembers[MemberIndex].Position);
    float Dist = ToCentroid.Size();

    if (Dist > HerdCohesionRadius)
    {
        return ToCentroid.GetSafeNormal();
    }
    return FVector::ZeroVector;
}

FVector ACrowdHerdMigration::CalculateAlignment(int32 MemberIndex) const
{
    if (!HerdMembers.IsValidIndex(MemberIndex)) return FVector::ZeroVector;

    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (int32 j = 0; j < HerdMembers.Num(); ++j)
    {
        if (j == MemberIndex || !HerdMembers[j].bIsAlive) continue;
        float Dist = FVector::Dist(HerdMembers[MemberIndex].Position, HerdMembers[j].Position);
        if (Dist < HerdCohesionRadius)
        {
            // Approximate velocity as direction toward waypoint
            if (MigrationPath.IsValidIndex(CurrentWaypointIndex))
            {
                AvgVelocity += (MigrationPath[CurrentWaypointIndex].Location - HerdMembers[j].Position).GetSafeNormal();
                Count++;
            }
        }
    }

    return Count > 0 ? (AvgVelocity / (float)Count).GetSafeNormal() : FVector::ZeroVector;
}

FVector ACrowdHerdMigration::CalculateAvoidanceVector(FVector FromLocation) const
{
    FVector TotalAvoidance = FVector::ZeroVector;

    for (const FCrowd_TerritoryZone& Zone : TerritoryZonesToAvoid)
    {
        float Dist = FVector::Dist(FromLocation, Zone.Center);
        float AvoidRadius = Zone.Radius * 1.5f;   // Start avoiding before entering zone

        if (Dist < AvoidRadius && Dist > 0.f)
        {
            FVector AwayDir = (FromLocation - Zone.Center).GetSafeNormal();
            float Strength = Zone.DangerLevel * (1.f - Dist / AvoidRadius);
            TotalAvoidance += AwayDir * Strength;
        }
    }

    return TotalAvoidance.GetSafeNormal();
}

void ACrowdHerdMigration::SetHerdState(ECrowd_HerdState NewState)
{
    CurrentState = NewState;
}

ECrowd_HerdState ACrowdHerdMigration::EvaluateStateTransition() const
{
    // Stampede overrides everything
    if (bStampedeActive && CurrentPanicLevel > 0.1f)
    {
        return ECrowd_HerdState::Stampeding;
    }

    // High panic → flee
    if (CurrentPanicLevel > 0.3f)
    {
        return ECrowd_HerdState::Fleeing;
    }

    // Keep current state if no override
    return CurrentState;
}

void ACrowdHerdMigration::KillMember(int32 MemberIndex)
{
    if (HerdMembers.IsValidIndex(MemberIndex))
    {
        HerdMembers[MemberIndex].bIsAlive = false;
        HerdMembers[MemberIndex].Health = 0.f;

        // Killing matriarch causes panic spike
        if (HerdMembers[MemberIndex].Role == ECrowd_HerdMemberRole::Matriarch)
        {
            CurrentPanicLevel = FMath::Min(1.f, CurrentPanicLevel + 0.5f);
        }
        else
        {
            CurrentPanicLevel = FMath::Min(1.f, CurrentPanicLevel + 0.2f);
        }
    }
}

int32 ACrowdHerdMigration::GetAliveCount() const
{
    int32 Count = 0;
    for (const FCrowd_HerdMember& M : HerdMembers)
    {
        if (M.bIsAlive) Count++;
    }
    return Count;
}

FVector ACrowdHerdMigration::GetMatriarchPosition() const
{
    for (const FCrowd_HerdMember& M : HerdMembers)
    {
        if (M.Role == ECrowd_HerdMemberRole::Matriarch && M.bIsAlive)
        {
            return M.Position;
        }
    }
    return GetHerdCentroid();
}
