#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestSurviveStampede.generated.h"

// ============================================================
// Quest: "Survive the Stampede"
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_001
// ============================================================

UENUM(BlueprintType)
enum class EQuest_StampedePhase : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    WarningIssued   UMETA(DisplayName = "Warning Issued"),
    StampedeActive  UMETA(DisplayName = "Stampede Active"),
    ReachHighGround UMETA(DisplayName = "Reach High Ground"),
    DefeatPredator  UMETA(DisplayName = "Defeat Predator"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FQuest_StampedeObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bOptional;

    FQuest_StampedeObjective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , bCompleted(false)
        , bOptional(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_StampedeReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 BoneTokens;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 StoneAxeCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 DriedMeatCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ReputationGain;

    FQuest_StampedeReward()
        : BoneTokens(3)
        , StoneAxeCount(1)
        , DriedMeatCount(5)
        , ReputationGain(25.0f)
    {}
};

/**
 * AQuestSurviveStampede
 * Quest actor placed in the world near the tribal elder NPC.
 * Triggers when player enters the quest radius and accepts.
 *
 * QUEST STRUCTURE:
 *   Phase 1 — Warning: Elder warns player of incoming stampede
 *   Phase 2 — Stampede Active: Herd begins fleeing south, danger zone active
 *   Phase 3 — Reach High Ground: Player must climb ridge (Z > 350) within 90s
 *   Phase 4 — Defeat Predator (optional): Drive off the TRex using fire/noise
 *   Phase 5 — Completed: Reward granted, elder dialogue plays
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestSurviveStampede : public AActor
{
    GENERATED_BODY()

public:
    AQuestSurviveStampede();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Quest State ──────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State")
    EQuest_StampedePhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float QuestTriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float HighGroundZThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float StampedeTimeLimit;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State")
    float ElapsedStampedeTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    FVector HighGroundTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    FVector StampedeDangerZoneMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    FVector StampedeDangerZoneMax;

    // ── Objectives ───────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Objectives")
    TArray<FQuest_StampedeObjective> Objectives;

    // ── Reward ───────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    FQuest_StampedeReward QuestReward;

    // ── Audio ─────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Audio")
    FString WarningVoiceLineURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Audio")
    FString CompleteVoiceLineURL;

    // ── Functions ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceToPhase(EQuest_StampedePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AreAllRequiredObjectivesComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsPlayerInDangerZone(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsPlayerOnHighGround(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    float GetStampedeTimeRemaining() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_StampedePhase GetCurrentPhase() const;

    UFUNCTION(CallInEditor, Category = "Quest|Debug")
    void DEBUG_StartQuestNow();

private:
    void InitialiseObjectives();
    void TickStampedePhase(float DeltaTime);
    bool bQuestStarted;
    bool bQuestEnded;
};
