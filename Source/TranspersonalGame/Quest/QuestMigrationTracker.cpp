#include "QuestMigrationTracker.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// Constructor
// ============================================================
AQuest_MigrationTracker::AQuest_MigrationTracker()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz — sufficient for quest logic

    // Default waypoints matching the crowd markers placed in MinPlayableMap
    // CrowdMigration_Path_01 → 02 → 03 → 04 → WaterSource
    FQuest_MigrationWaypoint WP0;
    WP0.WorldLocation = FVector(-5000.0f, -3000.0f, 100.0f);
    WP0.WaypointLabel = TEXT("Herd Gathering Point");
    WP0.AcceptanceRadius = 600.0f;
    MigrationWaypoints.Add(WP0);

    FQuest_MigrationWaypoint WP1;
    WP1.WorldLocation = FVector(-2000.0f, -1000.0f, 100.0f);
    WP1.WaypointLabel = TEXT("River Crossing");
    WP1.AcceptanceRadius = 500.0f;
    WP1.bTriggerStampedeOnArrive = true;
    MigrationWaypoints.Add(WP1);

    FQuest_MigrationWaypoint WP2;
    WP2.WorldLocation = FVector(1000.0f, 2000.0f, 100.0f);
    WP2.WaypointLabel = TEXT("Open Plains");
    WP2.AcceptanceRadius = 500.0f;
    MigrationWaypoints.Add(WP2);

    FQuest_MigrationWaypoint WP3;
    WP3.WorldLocation = FVector(4000.0f, 4000.0f, 100.0f);
    WP3.WaypointLabel = TEXT("Water Source");
    WP3.AcceptanceRadius = 700.0f;
    WP3.bIsWaterSource = true;
    MigrationWaypoints.Add(WP3);
}

// ============================================================
// BeginPlay
// ============================================================
void AQuest_MigrationTracker::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Ready. %d waypoints defined."), MigrationWaypoints.Num());
}

// ============================================================
// Tick
// ============================================================
void AQuest_MigrationTracker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bQuestActive) return;

    UpdateWaypointProgress(DeltaTime);
    UpdateStampedePhase(DeltaTime);
    CheckTimeLimit(DeltaTime);
}

// ============================================================
// StartMigrationQuest
// ============================================================
void AQuest_MigrationTracker::StartMigrationQuest()
{
    if (bQuestActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestMigrationTracker: Quest already active."));
        return;
    }

    MigrationState = FQuest_MigrationState();
    MigrationState.TimeLimit = 600.0f;
    MigrationState.CurrentWaypointIndex = 0;

    bQuestActive = true;
    SetPhase(EQuest_MigrationPhase::LocateHerd);

    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Migration quest STARTED. Objective: %s"),
        MigrationWaypoints.Num() > 0 ? *MigrationWaypoints[0].WaypointLabel : TEXT("Unknown"));
}

// ============================================================
// UpdateWaypointProgress
// ============================================================
void AQuest_MigrationTracker::UpdateWaypointProgress(float DeltaTime)
{
    if (MigrationState.CurrentPhase == EQuest_MigrationPhase::SurviveStampede) return;
    if (MigrationState.CurrentPhase == EQuest_MigrationPhase::Completed) return;
    if (MigrationState.CurrentPhase == EQuest_MigrationPhase::Failed) return;

    if (MigrationState.CurrentWaypointIndex >= MigrationWaypoints.Num()) return;

    // Get player location
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    FQuest_MigrationWaypoint& CurrentWP = MigrationWaypoints[MigrationState.CurrentWaypointIndex];

    float Dist = FVector::Dist(PlayerLoc, CurrentWP.WorldLocation);
    MigrationState.DistanceToNextWaypoint = Dist;

    // Phase: LocateHerd → FollowHerd when player is near first waypoint
    if (MigrationState.CurrentPhase == EQuest_MigrationPhase::LocateHerd)
    {
        if (Dist <= CurrentWP.AcceptanceRadius * 2.0f)
        {
            SetPhase(EQuest_MigrationPhase::FollowHerd);
        }
    }

    // Check if player reached current waypoint
    if (Dist <= CurrentWP.AcceptanceRadius)
    {
        CurrentWP.bReached = true;
        MigrationState.WaypointsReached++;

        UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Waypoint reached — %s"), *CurrentWP.WaypointLabel);

        // Trigger stampede if flagged
        if (CurrentWP.bTriggerStampedeOnArrive && !MigrationState.bStampedeActive)
        {
            TriggerStampedeEvent();
            return;
        }

        // Water source = quest complete
        if (CurrentWP.bIsWaterSource)
        {
            CompleteQuest();
            return;
        }

        AdvanceToNextWaypoint();
    }
}

// ============================================================
// AdvanceToNextWaypoint
// ============================================================
void AQuest_MigrationTracker::AdvanceToNextWaypoint()
{
    MigrationState.CurrentWaypointIndex++;

    if (MigrationState.CurrentWaypointIndex >= MigrationWaypoints.Num())
    {
        CompleteQuest();
        return;
    }

    FQuest_MigrationWaypoint& Next = MigrationWaypoints[MigrationState.CurrentWaypointIndex];
    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Next objective — %s"), *Next.WaypointLabel);
}

// ============================================================
// TriggerStampedeEvent
// ============================================================
void AQuest_MigrationTracker::TriggerStampedeEvent()
{
    MigrationState.bStampedeActive = true;
    StampedeElapsed = 0.0f;
    SetPhase(EQuest_MigrationPhase::SurviveStampede);

    OnStampedeTriggered.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("QuestMigrationTracker: STAMPEDE TRIGGERED! Player must survive for %.1fs"), StampedeWarningTime);
}

// ============================================================
// UpdateStampedePhase
// ============================================================
void AQuest_MigrationTracker::UpdateStampedePhase(float DeltaTime)
{
    if (!MigrationState.bStampedeActive) return;

    StampedeElapsed += DeltaTime;

    // Check if player is in danger zone
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (PC && PC->GetPawn())
    {
        FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
        FVector StampedeOrigin = MigrationWaypoints[MigrationState.CurrentWaypointIndex].WorldLocation;
        float DistFromStampede = FVector::Dist(PlayerLoc, StampedeOrigin);

        if (DistFromStampede < StampedeDangerRadius)
        {
            // Player is in danger — damage is applied by gameplay systems
            UE_LOG(LogTemp, Warning, TEXT("QuestMigrationTracker: Player in stampede danger zone! Dist=%.0f"), DistFromStampede);
        }
    }

    // Stampede ends after warning time — player survived
    if (StampedeElapsed >= StampedeWarningTime)
    {
        MigrationState.bStampedeActive = false;
        UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Stampede survived! Continuing migration."));
        SetPhase(EQuest_MigrationPhase::FollowHerd);
        AdvanceToNextWaypoint();
    }
}

// ============================================================
// CheckTimeLimit
// ============================================================
void AQuest_MigrationTracker::CheckTimeLimit(float DeltaTime)
{
    MigrationState.ElapsedTime += DeltaTime;

    if (MigrationState.ElapsedTime >= MigrationState.TimeLimit)
    {
        FailQuest(EQuest_MigrationFailReason::TimeExpired);
    }
}

// ============================================================
// CompleteQuest
// ============================================================
void AQuest_MigrationTracker::CompleteQuest()
{
    bQuestActive = false;
    SetPhase(EQuest_MigrationPhase::Completed);
    OnQuestCompleted.Broadcast(MigrationState.ElapsedTime);

    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: QUEST COMPLETE! Time: %.1fs, Waypoints: %d/%d"),
        MigrationState.ElapsedTime,
        MigrationState.WaypointsReached,
        MigrationWaypoints.Num());
}

// ============================================================
// FailQuest
// ============================================================
void AQuest_MigrationTracker::FailQuest(EQuest_MigrationFailReason Reason)
{
    bQuestActive = false;
    MigrationState.FailReason = Reason;
    SetPhase(EQuest_MigrationPhase::Failed);

    UE_LOG(LogTemp, Warning, TEXT("QuestMigrationTracker: QUEST FAILED — Reason: %d"), (int32)Reason);
}

// ============================================================
// GetCurrentWaypoint
// ============================================================
FQuest_MigrationWaypoint AQuest_MigrationTracker::GetCurrentWaypoint() const
{
    if (MigrationState.CurrentWaypointIndex < MigrationWaypoints.Num())
    {
        return MigrationWaypoints[MigrationState.CurrentWaypointIndex];
    }
    return FQuest_MigrationWaypoint();
}

// ============================================================
// GetProgressPercent
// ============================================================
float AQuest_MigrationTracker::GetProgressPercent() const
{
    if (MigrationWaypoints.Num() == 0) return 0.0f;
    return (float)MigrationState.WaypointsReached / (float)MigrationWaypoints.Num();
}

// ============================================================
// IsPlayerNearHerd
// ============================================================
bool AQuest_MigrationTracker::IsPlayerNearHerd(const FVector& PlayerLocation) const
{
    if (MigrationState.CurrentWaypointIndex >= MigrationWaypoints.Num()) return false;

    const FQuest_MigrationWaypoint& WP = MigrationWaypoints[MigrationState.CurrentWaypointIndex];
    float Dist = FVector::Dist(PlayerLocation, WP.WorldLocation);
    return Dist <= HerdProximityRadius;
}

// ============================================================
// SetPhase
// ============================================================
void AQuest_MigrationTracker::SetPhase(EQuest_MigrationPhase NewPhase)
{
    MigrationState.CurrentPhase = NewPhase;
    OnMigrationPhaseChanged.Broadcast(NewPhase);
    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Phase → %d"), (int32)NewPhase);
}

// ============================================================
// DEBUG_StartQuestInEditor
// ============================================================
void AQuest_MigrationTracker::DEBUG_StartQuestInEditor()
{
    UE_LOG(LogTemp, Warning, TEXT("QuestMigrationTracker: DEBUG — Starting quest from editor."));
    StartMigrationQuest();
}
