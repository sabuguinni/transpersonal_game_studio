// QuestMigrationTracker.cpp
// Quest & Mission Designer — Agent #14
// Full implementation of the Migration Quest Tracker component

#include "QuestMigrationTracker.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

UQuestMigrationTracker::UQuestMigrationTracker()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // 2Hz update

    CurrentPhase = EQuest_MigrationPhase::NotStarted;
    FailReason = EQuest_MigrationFailReason::None;
    QuestTimeLimit = 600.0f; // 10 minutes
    ElapsedTime = 0.0f;
    WaypointProximityRadius = 400.0f;
    MaxHerdDistance = 2500.0f;
    bQuestActive = false;
    CurrentWaypointIndex = 0;
}

void UQuestMigrationTracker::BeginPlay()
{
    Super::BeginPlay();
}

void UQuestMigrationTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bQuestActive) return;

    ElapsedTime += DeltaTime;

    // Time limit check
    if (ElapsedTime >= QuestTimeLimit)
    {
        FailQuest(EQuest_MigrationFailReason::TimeExpired);
        return;
    }

    UpdatePhaseLogic();
}

void UQuestMigrationTracker::StartMigrationQuest()
{
    if (bQuestActive) return;

    bQuestActive = true;
    CurrentPhase = EQuest_MigrationPhase::LocateHerd;
    ElapsedTime = 0.0f;
    CurrentWaypointIndex = 0;
    FailReason = EQuest_MigrationFailReason::None;

    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Migration quest started — Phase: LocateHerd"));
    OnPhaseChanged.Broadcast(CurrentPhase);
}

void UQuestMigrationTracker::UpdatePhaseLogic()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector PlayerLocation = Owner->GetActorLocation();

    switch (CurrentPhase)
    {
        case EQuest_MigrationPhase::LocateHerd:
        {
            // Check if player is within range of herd anchor waypoint
            if (Waypoints.IsValidIndex(CurrentWaypointIndex))
            {
                float Dist = FVector::Dist(PlayerLocation, Waypoints[CurrentWaypointIndex].WorldLocation);
                if (Dist <= WaypointProximityRadius)
                {
                    AdvancePhase(EQuest_MigrationPhase::FollowHerd);
                }
            }
            break;
        }

        case EQuest_MigrationPhase::FollowHerd:
        {
            // Monitor herd distance — fail if player strays too far
            if (HerdAnchorActor.IsValid())
            {
                float HerdDist = FVector::Dist(PlayerLocation, HerdAnchorActor->GetActorLocation());
                if (HerdDist > MaxHerdDistance)
                {
                    FailQuest(EQuest_MigrationFailReason::HerdLost);
                    return;
                }
            }

            // Advance to next waypoint
            if (Waypoints.IsValidIndex(CurrentWaypointIndex))
            {
                float Dist = FVector::Dist(PlayerLocation, Waypoints[CurrentWaypointIndex].WorldLocation);
                if (Dist <= WaypointProximityRadius)
                {
                    CurrentWaypointIndex++;
                    if (CurrentWaypointIndex >= Waypoints.Num())
                    {
                        AdvancePhase(EQuest_MigrationPhase::ReachWaterSource);
                    }
                }
            }
            break;
        }

        case EQuest_MigrationPhase::ReachWaterSource:
        {
            // Check proximity to water source waypoint
            if (WaterSourceLocation != FVector::ZeroVector)
            {
                float Dist = FVector::Dist(PlayerLocation, WaterSourceLocation);
                if (Dist <= WaypointProximityRadius * 1.5f)
                {
                    AdvancePhase(EQuest_MigrationPhase::SurviveStampede);
                }
            }
            break;
        }

        case EQuest_MigrationPhase::SurviveStampede:
        {
            // Stampede survival is event-driven — handled by external trigger
            // This phase completes via CompleteStampedePhase()
            break;
        }

        default:
            break;
    }
}

void UQuestMigrationTracker::AdvancePhase(EQuest_MigrationPhase NewPhase)
{
    CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Phase advanced to %d"), (int32)NewPhase);
    OnPhaseChanged.Broadcast(NewPhase);
}

void UQuestMigrationTracker::CompleteStampedePhase()
{
    if (CurrentPhase != EQuest_MigrationPhase::SurviveStampede) return;

    AdvancePhase(EQuest_MigrationPhase::Completed);
    bQuestActive = false;
    OnQuestCompleted.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("QuestMigrationTracker: Quest COMPLETED!"));
}

void UQuestMigrationTracker::FailQuest(EQuest_MigrationFailReason Reason)
{
    if (!bQuestActive) return;

    bQuestActive = false;
    FailReason = Reason;
    CurrentPhase = EQuest_MigrationPhase::Failed;

    UE_LOG(LogTemp, Warning, TEXT("QuestMigrationTracker: Quest FAILED — Reason: %d"), (int32)Reason);
    OnQuestFailed.Broadcast(Reason);
}

void UQuestMigrationTracker::SetHerdAnchor(AActor* HerdActor)
{
    HerdAnchorActor = HerdActor;
}

void UQuestMigrationTracker::SetWaterSourceLocation(FVector Location)
{
    WaterSourceLocation = Location;
}

void UQuestMigrationTracker::AddWaypoint(FQuest_MigrationWaypoint Waypoint)
{
    Waypoints.Add(Waypoint);
}

EQuest_MigrationPhase UQuestMigrationTracker::GetCurrentPhase() const
{
    return CurrentPhase;
}

float UQuestMigrationTracker::GetQuestProgress() const
{
    if (!bQuestActive) return 0.0f;
    int32 TotalPhases = 4; // LocateHerd, FollowHerd, ReachWaterSource, SurviveStampede
    int32 PhaseIndex = FMath::Clamp((int32)CurrentPhase - 1, 0, TotalPhases);
    return (float)PhaseIndex / (float)TotalPhases;
}
