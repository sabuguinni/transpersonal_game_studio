// QuestStampedeTracker.h
// Agent #14 — Quest & Mission Designer
// CYCLE: PROD_CYCLE_AUTO_20260629_006
//
// Quest: "Track the Migration" + "Survive the Stampede"
// Wires UCrowdStampedeController events to quest objectives.
// Uses EQuest_Status, EQuest_ObjectiveType from QuestSystemTypes.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Quest/QuestSystemTypes.h"

// Forward declare crowd controller — do NOT include to avoid cross-module dependency
class UCrowdStampedeController;

#include "QuestStampedeTracker.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest14_StampedeQuestPhase : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    ApproachHerd    UMETA(DisplayName = "Approach the Herd"),
    TrackMigration  UMETA(DisplayName = "Track Migration Path"),
    StampedeActive  UMETA(DisplayName = "Stampede Active - Survive!"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest14_MigrationWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    float TriggerRadius = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    bool bReached = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    FString WaypointLabel = TEXT("Waypoint");
};

USTRUCT(BlueprintType)
struct FQuest14_StampedeQuestData
{
    GENERATED_BODY()

    // Quest identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestID = FName("Q14_TrackMigration");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle = FText::FromString(TEXT("Track the Migration"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription = FText::FromString(
        TEXT("The Triceratops herd moves south every season. Follow their migration path "
             "through the valley — but beware the predator that follows them."));

    // Objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 WaypointsRequired = 5;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 WaypointsReached = 0;

    // Survival condition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float StampedeSurvivalTimeRequired = 30.0f; // seconds

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float StampedeSurvivalTimeElapsed = 0.0f;

    // Reward
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward CompletionReward;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest14_StampedeQuestPhase CurrentPhase = EQuest14_StampedeQuestPhase::NotStarted;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_Status QuestStatus = EQuest_Status::Inactive;
};

// ─── Actor ───────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "TranspersonalGame|Quest", meta = (DisplayName = "Quest: Stampede Tracker"))
class TRANSPERSONALGAME_API AQuestStampedeTracker : public AActor
{
    GENERATED_BODY()

public:
    AQuestStampedeTracker();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Quest API (Blueprint-callable) ────────────────────────────────────

    /** Activate this quest — call from PlayerController or trigger volume */
    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void ActivateQuest();

    /** Called when player reaches a migration waypoint */
    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void OnWaypointReached(int32 WaypointIndex);

    /** Called when UCrowdStampedeController fires TriggerStampede */
    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void OnStampedeTriggered(FVector Epicentre, float Intensity);

    /** Called every frame while stampede is active — checks player danger */
    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void TickStampedeSurvival(float DeltaTime, bool bPlayerInDanger);

    /** Fail the quest — player was trampled */
    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void FailQuest(FText Reason);

    /** Complete the quest — all objectives met */
    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void CompleteQuest();

    /** Returns current quest phase as string for HUD display */
    UFUNCTION(BlueprintPure, Category = "Quest|Stampede")
    FText GetCurrentObjectiveText() const;

    /** Returns 0.0–1.0 progress through migration waypoints */
    UFUNCTION(BlueprintPure, Category = "Quest|Stampede")
    float GetMigrationProgress() const;

    /** Returns true if stampede survival timer is running */
    UFUNCTION(BlueprintPure, Category = "Quest|Stampede")
    bool IsInStampedePhase() const;

    // ── Properties ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Data")
    FQuest14_StampedeQuestData QuestData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Waypoints")
    TArray<FQuest14_MigrationWaypoint> MigrationWaypoints;

    /** Radius around player to detect herd approach and trigger quest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Trigger")
    float HerdApproachTriggerRadius = 800.0f;

    /** Herd panic level threshold to trigger stampede quest phase (0.0–1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Trigger")
    float PanicLevelThreshold = 0.6f;

    /** Debug: draw waypoint spheres in editor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Debug")
    bool bDrawDebugWaypoints = true;

private:
    // Internal state
    bool bStampedePhaseActive = false;
    float TimeSinceQuestActivated = 0.0f;

    // Tick helpers
    void TickWaypointDetection(float DeltaTime);
    void TickDebugDraw();
    void AdvanceToPhase(EQuest14_StampedeQuestPhase NewPhase);
    FText PhaseToObjectiveText(EQuest14_StampedeQuestPhase Phase) const;
};
