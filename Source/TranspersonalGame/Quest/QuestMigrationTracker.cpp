// QuestMigrationTracker.cpp
// Agent #14 — Quest & Mission Designer
// Migration quest: follow the herd to the river crossing before dry season

#include "QuestMigrationTracker.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UQuestMigrationTracker::UQuestMigrationTracker()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // 2Hz update

    CurrentPhase = EQuest_MigrationPhase::NotStarted;
    FailReason = EQuest_MigrationFailReason::None;
    bQuestActive = false;
    bQuestCompleted = false;
    GlobalPanicLevel = 0.0f;
    TimeRemainingSeconds = 600.0f; // 10 minutes default
    QuestTimeLimit = 600.0f;
    HerdLeaderLocation = FVector::ZeroVector;
    MaxDistanceFromHerd = 2500.0f;
    WaterSourceLocation = FVector(8000.0f, 3000.0f, 50.0f);
    StampedeZoneRadius = 1500.0f;
    WaypointReachRadius = 400.0f;
    CurrentWaypointIndex = 0;
}

void UQuestMigrationTracker::BeginPlay()
{
    Super::BeginPlay();
    SetupDefaultWaypoints();
}

void UQuestMigrationTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bQuestActive || bQuestCompleted)
        return;

    UpdateQuestTimer(DeltaTime);
    UpdatePhaseLogic(DeltaTime);
}

void UQuestMigrationTracker::SetupDefaultWaypoints()
{
    Waypoints.Empty();

    // Waypoint 0 — Herd gathering point
    FQuest_MigrationWaypoint WP0;
    WP0.WorldLocation = FVector(3000.0f, 1500.0f, 100.0f);
    WP0.WaypointLabel = TEXT("Locate the Herd");
    WP0.bIsStampedeZone = false;
    WP0.bIsWaterSource = false;
    WP0.RequiredPhase = EQuest_MigrationPhase::LocateHerd;
    WP0.DangerRadius = 0.0f;
    Waypoints.Add(WP0);

    // Waypoint 1 — Migration column start
    FQuest_MigrationWaypoint WP1;
    WP1.WorldLocation = FVector(1000.0f, 800.0f, 100.0f);
    WP1.WaypointLabel = TEXT("Join the Migration");
    WP1.bIsStampedeZone = false;
    WP1.bIsWaterSource = false;
    WP1.RequiredPhase = EQuest_MigrationPhase::FollowHerd;
    WP1.DangerRadius = 0.0f;
    Waypoints.Add(WP1);

    // Waypoint 2 — Narrow canyon (stampede risk)
    FQuest_MigrationWaypoint WP2;
    WP2.WorldLocation = FVector(4500.0f, 2200.0f, 100.0f);
    WP2.WaypointLabel = TEXT("Cross the Canyon");
    WP2.bIsStampedeZone = true;
    WP2.bIsWaterSource = false;
    WP2.RequiredPhase = EQuest_MigrationPhase::SurviveStampede;
    WP2.DangerRadius = StampedeZoneRadius;
    Waypoints.Add(WP2);

    // Waypoint 3 — River crossing / water source
    FQuest_MigrationWaypoint WP3;
    WP3.WorldLocation = WaterSourceLocation;
    WP3.WaypointLabel = TEXT("Reach the River");
    WP3.bIsStampedeZone = false;
    WP3.bIsWaterSource = true;
    WP3.RequiredPhase = EQuest_MigrationPhase::ReachWaterSource;
    WP3.DangerRadius = 0.0f;
    Waypoints.Add(WP3);
}

void UQuestMigrationTracker::StartMigrationQuest()
{
    if (bQuestActive)
        return;

    bQuestActive = true;
    bQuestCompleted = false;
    CurrentPhase = EQuest_MigrationPhase::LocateHerd;
    FailReason = EQuest_MigrationFailReason::None;
    TimeRemainingSeconds = QuestTimeLimit;
    CurrentWaypointIndex = 0;
    GlobalPanicLevel = 0.0f;

    OnQuestPhaseChanged.Broadcast(CurrentPhase);
    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Migration quest STARTED"));
}

void UQuestMigrationTracker::UpdateQuestTimer(float DeltaTime)
{
    if (TimeRemainingSeconds <= 0.0f)
    {
        FailQuest(EQuest_MigrationFailReason::TimeExpired);
        return;
    }
    TimeRemainingSeconds -= DeltaTime;
}

void UQuestMigrationTracker::UpdatePhaseLogic(float DeltaTime)
{
    if (Waypoints.Num() == 0 || CurrentWaypointIndex >= Waypoints.Num())
        return;

    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    FVector PlayerLoc = Owner->GetActorLocation();
    FQuest_MigrationWaypoint& CurrentWP = Waypoints[CurrentWaypointIndex];

    float DistToWaypoint = FVector::Dist(PlayerLoc, CurrentWP.WorldLocation);

    // Check stampede zone danger
    if (CurrentWP.bIsStampedeZone && CurrentPhase == EQuest_MigrationPhase::SurviveStampede)
    {
        float PanicAtPlayer = GetPanicLevelAtLocation(PlayerLoc);
        if (PanicAtPlayer > 0.85f)
        {
            // High panic = trample risk
            OnStampedeWarning.Broadcast(PanicAtPlayer);
        }
    }

    // Check herd distance (FollowHerd phase)
    if (CurrentPhase == EQuest_MigrationPhase::FollowHerd)
    {
        float DistToHerd = FVector::Dist(PlayerLoc, HerdLeaderLocation);
        if (DistToHerd > MaxDistanceFromHerd && HerdLeaderLocation != FVector::ZeroVector)
        {
            // Player fell too far behind — warn but don't fail immediately
            OnHerdDistanceWarning.Broadcast(DistToHerd);
        }
    }

    // Waypoint reached?
    if (DistToWaypoint <= WaypointReachRadius)
    {
        AdvanceToNextWaypoint();
    }
}

void UQuestMigrationTracker::AdvanceToNextWaypoint()
{
    CurrentWaypointIndex++;

    if (CurrentWaypointIndex >= Waypoints.Num())
    {
        // All waypoints reached — complete quest
        CompleteQuest();
        return;
    }

    // Advance phase
    EQuest_MigrationPhase NextPhase = Waypoints[CurrentWaypointIndex].RequiredPhase;
    if (NextPhase != CurrentPhase)
    {
        CurrentPhase = NextPhase;
        OnQuestPhaseChanged.Broadcast(CurrentPhase);
        UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Phase advanced to %d"), (int32)CurrentPhase);
    }
}

void UQuestMigrationTracker::CompleteQuest()
{
    bQuestActive = false;
    bQuestCompleted = true;
    CurrentPhase = EQuest_MigrationPhase::Completed;
    OnQuestPhaseChanged.Broadcast(CurrentPhase);
    OnQuestCompleted.Broadcast(true, FailReason);
    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Migration quest COMPLETED"));
}

void UQuestMigrationTracker::FailQuest(EQuest_MigrationFailReason Reason)
{
    bQuestActive = false;
    FailReason = Reason;
    CurrentPhase = EQuest_MigrationPhase::Failed;
    OnQuestPhaseChanged.Broadcast(CurrentPhase);
    OnQuestCompleted.Broadcast(false, Reason);
    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Migration quest FAILED — reason %d"), (int32)Reason);
}

float UQuestMigrationTracker::GetPanicLevelAtLocation(FVector Location) const
{
    // Combine global panic with proximity to stampede waypoints
    float LocalPanic = GlobalPanicLevel;

    for (const FQuest_MigrationWaypoint& WP : Waypoints)
    {
        if (!WP.bIsStampedeZone || WP.DangerRadius <= 0.0f)
            continue;

        float Dist = FVector::Dist(Location, WP.WorldLocation);
        if (Dist < WP.DangerRadius)
        {
            float ZonePanic = 1.0f - (Dist / WP.DangerRadius);
            LocalPanic = FMath::Max(LocalPanic, ZonePanic);
        }
    }

    return FMath::Clamp(LocalPanic, 0.0f, 1.0f);
}

void UQuestMigrationTracker::UpdateHerdLeaderLocation(FVector NewLocation)
{
    HerdLeaderLocation = NewLocation;
}

void UQuestMigrationTracker::SetGlobalPanicLevel(float PanicLevel)
{
    GlobalPanicLevel = FMath::Clamp(PanicLevel, 0.0f, 1.0f);
}

FQuest_MigrationWaypoint UQuestMigrationTracker::GetCurrentWaypoint() const
{
    if (CurrentWaypointIndex < Waypoints.Num())
        return Waypoints[CurrentWaypointIndex];

    return FQuest_MigrationWaypoint();
}

float UQuestMigrationTracker::GetProgressPercent() const
{
    if (Waypoints.Num() == 0)
        return 0.0f;

    return FMath::Clamp((float)CurrentWaypointIndex / (float)Waypoints.Num(), 0.0f, 1.0f);
}
