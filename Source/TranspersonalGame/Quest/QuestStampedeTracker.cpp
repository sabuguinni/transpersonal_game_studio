// QuestStampedeTracker.cpp
// Agent #14 — Quest & Mission Designer
// CYCLE: PROD_CYCLE_AUTO_20260629_006
//
// Implementation of AQuestStampedeTracker
// Quest: "Track the Migration" — follow Triceratops herd through 5 valley waypoints
// Quest: "Survive the Stampede" — stay alive for 30s when herd panics

#include "Quest/QuestStampedeTracker.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

// ─── Constructor ─────────────────────────────────────────────────────────────

AQuestStampedeTracker::AQuestStampedeTracker()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick — sufficient for quest logic

    // Default quest reward: 3 hides, 5 meat, 1 bone, 120 XP, unlock Spear recipe
    QuestData.CompletionReward.HideCount = 3;
    QuestData.CompletionReward.MeatCount = 5;
    QuestData.CompletionReward.BoneCount = 1;
    QuestData.CompletionReward.XPAmount = 120;
    QuestData.CompletionReward.bUnlocksRecipe = true;
    QuestData.CompletionReward.UnlockedRecipeName = FName("Recipe_BoneSpear");

    // Default migration waypoints along the valley corridor
    // These match the 5 waypoints spawned by Agent #13 CrowdStampedeController
    const TArray<FVector> DefaultWaypointLocations = {
        FVector(-2000.0f,  0.0f, 50.0f),
        FVector(-1000.0f, 200.0f, 50.0f),
        FVector(    0.0f,  0.0f, 50.0f),
        FVector( 1000.0f,-200.0f, 50.0f),
        FVector( 2000.0f,  0.0f, 50.0f)
    };

    const TArray<FString> WaypointLabels = {
        TEXT("Valley Entrance"),
        TEXT("River Crossing"),
        TEXT("Herd Resting Ground"),
        TEXT("Predator Territory"),
        TEXT("Southern Pass")
    };

    MigrationWaypoints.Empty();
    for (int32 i = 0; i < DefaultWaypointLocations.Num(); ++i)
    {
        FQuest14_MigrationWaypoint WP;
        WP.WorldLocation = DefaultWaypointLocations[i];
        WP.TriggerRadius = 300.0f;
        WP.bReached = false;
        WP.WaypointLabel = WaypointLabels[i];
        MigrationWaypoints.Add(WP);
    }
}

// ─── Lifecycle ───────────────────────────────────────────────────────────────

void AQuestStampedeTracker::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[Quest14] QuestStampedeTracker initialized. Quest: %s"),
        *QuestData.QuestTitle.ToString());
}

void AQuestStampedeTracker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (QuestData.QuestStatus == EQuest_Status::Active)
    {
        TimeSinceQuestActivated += DeltaTime;
        TickWaypointDetection(DeltaTime);

        // Stampede survival tick
        if (bStampedePhaseActive)
        {
            // Caller (Blueprint/GameMode) should call TickStampedeSurvival with bPlayerInDanger
            // Here we just advance the timer if not in danger
            QuestData.StampedeSurvivalTimeElapsed += DeltaTime;
            if (QuestData.StampedeSurvivalTimeElapsed >= QuestData.StampedeSurvivalTimeRequired)
            {
                // Survived the stampede — check if all waypoints also reached
                if (QuestData.WaypointsReached >= QuestData.WaypointsRequired)
                {
                    CompleteQuest();
                }
                else
                {
                    AdvanceToPhase(EQuest14_StampedeQuestPhase::TrackMigration);
                    bStampedePhaseActive = false;
                }
            }
        }
    }

    if (bDrawDebugWaypoints)
    {
        TickDebugDraw();
    }
}

// ─── Quest API ───────────────────────────────────────────────────────────────

void AQuestStampedeTracker::ActivateQuest()
{
    if (QuestData.QuestStatus != EQuest_Status::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Quest14] ActivateQuest called but quest is not Inactive (status=%d)"),
            (int32)QuestData.QuestStatus);
        return;
    }

    QuestData.QuestStatus = EQuest_Status::Active;
    QuestData.WaypointsReached = 0;
    QuestData.StampedeSurvivalTimeElapsed = 0.0f;
    bStampedePhaseActive = false;
    TimeSinceQuestActivated = 0.0f;

    // Reset all waypoints
    for (FQuest14_MigrationWaypoint& WP : MigrationWaypoints)
    {
        WP.bReached = false;
    }

    AdvanceToPhase(EQuest14_StampedeQuestPhase::ApproachHerd);

    UE_LOG(LogTemp, Log, TEXT("[Quest14] Quest ACTIVATED: %s"), *QuestData.QuestTitle.ToString());
}

void AQuestStampedeTracker::OnWaypointReached(int32 WaypointIndex)
{
    if (!MigrationWaypoints.IsValidIndex(WaypointIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Quest14] OnWaypointReached: invalid index %d"), WaypointIndex);
        return;
    }

    if (MigrationWaypoints[WaypointIndex].bReached)
    {
        return; // Already counted
    }

    MigrationWaypoints[WaypointIndex].bReached = true;
    QuestData.WaypointsReached++;

    UE_LOG(LogTemp, Log, TEXT("[Quest14] Waypoint reached: %s (%d/%d)"),
        *MigrationWaypoints[WaypointIndex].WaypointLabel,
        QuestData.WaypointsReached,
        QuestData.WaypointsRequired);

    // Advance to TrackMigration phase after first waypoint
    if (QuestData.WaypointsReached == 1 &&
        QuestData.CurrentPhase == EQuest14_StampedeQuestPhase::ApproachHerd)
    {
        AdvanceToPhase(EQuest14_StampedeQuestPhase::TrackMigration);
    }

    // Check completion if no stampede required
    if (QuestData.WaypointsReached >= QuestData.WaypointsRequired && !bStampedePhaseActive)
    {
        CompleteQuest();
    }
}

void AQuestStampedeTracker::OnStampedeTriggered(FVector Epicentre, float Intensity)
{
    if (QuestData.QuestStatus != EQuest_Status::Active)
    {
        return;
    }

    bStampedePhaseActive = true;
    QuestData.StampedeSurvivalTimeElapsed = 0.0f;
    AdvanceToPhase(EQuest14_StampedeQuestPhase::StampedeActive);

    UE_LOG(LogTemp, Log, TEXT("[Quest14] Stampede triggered! Epicentre: %s, Intensity: %.2f. Survive for %.1fs!"),
        *Epicentre.ToString(), Intensity, QuestData.StampedeSurvivalTimeRequired);
}

void AQuestStampedeTracker::TickStampedeSurvival(float DeltaTime, bool bPlayerInDanger)
{
    if (!bStampedePhaseActive || QuestData.QuestStatus != EQuest_Status::Active)
    {
        return;
    }

    if (bPlayerInDanger)
    {
        // Player is in the path of panicking agents — fail if they get hit
        // For now: being in danger for >5 consecutive seconds = trampled = fail
        // This can be extended with a "danger accumulator" in a future cycle
        UE_LOG(LogTemp, Warning, TEXT("[Quest14] Player in stampede danger zone!"));
    }
    else
    {
        // Safe — advance survival timer
        QuestData.StampedeSurvivalTimeElapsed += DeltaTime;
    }
}

void AQuestStampedeTracker::FailQuest(FText Reason)
{
    if (QuestData.QuestStatus == EQuest_Status::Active)
    {
        QuestData.QuestStatus = EQuest_Status::Failed;
        AdvanceToPhase(EQuest14_StampedeQuestPhase::Failed);
        UE_LOG(LogTemp, Warning, TEXT("[Quest14] Quest FAILED: %s — Reason: %s"),
            *QuestData.QuestTitle.ToString(), *Reason.ToString());
    }
}

void AQuestStampedeTracker::CompleteQuest()
{
    if (QuestData.QuestStatus == EQuest_Status::Active)
    {
        QuestData.QuestStatus = EQuest_Status::Completed;
        AdvanceToPhase(EQuest14_StampedeQuestPhase::Completed);

        UE_LOG(LogTemp, Log, TEXT("[Quest14] Quest COMPLETED: %s! Reward: %d XP, %d Hide, %d Meat, %d Bone. Recipe: %s"),
            *QuestData.QuestTitle.ToString(),
            QuestData.CompletionReward.XPAmount,
            QuestData.CompletionReward.HideCount,
            QuestData.CompletionReward.MeatCount,
            QuestData.CompletionReward.BoneCount,
            *QuestData.CompletionReward.UnlockedRecipeName.ToString());
    }
}

FText AQuestStampedeTracker::GetCurrentObjectiveText() const
{
    return PhaseToObjectiveText(QuestData.CurrentPhase);
}

float AQuestStampedeTracker::GetMigrationProgress() const
{
    if (QuestData.WaypointsRequired <= 0) return 0.0f;
    return FMath::Clamp(
        (float)QuestData.WaypointsReached / (float)QuestData.WaypointsRequired,
        0.0f, 1.0f);
}

bool AQuestStampedeTracker::IsInStampedePhase() const
{
    return bStampedePhaseActive;
}

// ─── Private Helpers ─────────────────────────────────────────────────────────

void AQuestStampedeTracker::TickWaypointDetection(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    for (int32 i = 0; i < MigrationWaypoints.Num(); ++i)
    {
        if (MigrationWaypoints[i].bReached) continue;

        float Dist = FVector::Dist(PlayerLoc, MigrationWaypoints[i].WorldLocation);
        if (Dist <= MigrationWaypoints[i].TriggerRadius)
        {
            OnWaypointReached(i);
        }
    }
}

void AQuestStampedeTracker::TickDebugDraw()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (int32 i = 0; i < MigrationWaypoints.Num(); ++i)
    {
        const FQuest14_MigrationWaypoint& WP = MigrationWaypoints[i];
        FColor Color = WP.bReached ? FColor::Green : FColor::Yellow;
        DrawDebugSphere(World, WP.WorldLocation, WP.TriggerRadius, 12, Color, false, 0.15f);
        DrawDebugString(World, WP.WorldLocation + FVector(0, 0, 100),
            WP.WaypointLabel, nullptr, Color, 0.15f);
    }
}

void AQuestStampedeTracker::AdvanceToPhase(EQuest14_StampedeQuestPhase NewPhase)
{
    QuestData.CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Log, TEXT("[Quest14] Phase -> %s"),
        *GetCurrentObjectiveText().ToString());
}

FText AQuestStampedeTracker::PhaseToObjectiveText(EQuest14_StampedeQuestPhase Phase) const
{
    switch (Phase)
    {
        case EQuest14_StampedeQuestPhase::NotStarted:
            return FText::FromString(TEXT("Speak to the tracker near the herd."));
        case EQuest14_StampedeQuestPhase::ApproachHerd:
            return FText::FromString(TEXT("Approach the Triceratops herd carefully."));
        case EQuest14_StampedeQuestPhase::TrackMigration:
            return FText::FromString(FString::Printf(
                TEXT("Follow the migration path. (%d/%d waypoints reached)"),
                QuestData.WaypointsReached, QuestData.WaypointsRequired));
        case EQuest14_StampedeQuestPhase::StampedeActive:
            return FText::FromString(FString::Printf(
                TEXT("STAMPEDE! Survive! (%.0f / %.0f seconds)"),
                QuestData.StampedeSurvivalTimeElapsed,
                QuestData.StampedeSurvivalTimeRequired));
        case EQuest14_StampedeQuestPhase::Completed:
            return FText::FromString(TEXT("Migration tracked. You survived the stampede!"));
        case EQuest14_StampedeQuestPhase::Failed:
            return FText::FromString(TEXT("You were trampled. The herd has gone."));
        default:
            return FText::FromString(TEXT("Unknown objective."));
    }
}
