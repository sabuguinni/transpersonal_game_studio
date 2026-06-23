// QuestManager.h
// Quest & Tutorial System — Agent #04 Performance Optimizer | Cycle 013
// Prehistoric survival game — NO spiritual/mystical content

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_QuestState : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EPerf_QuestType : uint8
{
    Tutorial    UMETA(DisplayName = "Tutorial"),
    Survival    UMETA(DisplayName = "Survival"),
    Exploration UMETA(DisplayName = "Exploration"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Crafting    UMETA(DisplayName = "Crafting")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FPerf_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ProximityRadius = 500.0f;
};

USTRUCT(BlueprintType)
struct FPerf_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EPerf_QuestType QuestType = EPerf_QuestType::Tutorial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EPerf_QuestState State = EPerf_QuestState::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FPerf_QuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f; // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest = false;
};

// ─── Delegates ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnQuestActivated, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnQuestCompleted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnQuestFailed, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPerf_OnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);

// ─── QuestManager Actor ───────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API APerf_QuestManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_QuestManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Quest Registry ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests")
    TArray<FPerf_QuestData> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quests")
    FString ActiveQuestID;

    UPROPERTY(BlueprintReadOnly, Category = "Quests")
    TArray<FString> CompletedQuestIDs;

    // ── Tutorial Quest ────────────────────────────────────────────────────────

    /** Location of the water source for the tutorial quest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FVector TutorialWaterLocation = FVector(8000.0f, 0.0f, 0.0f);

    /** Radius within which the player is considered to have found water */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float TutorialWaterRadius = 800.0f;

    // ── Delegates ─────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FPerf_OnQuestActivated OnQuestActivated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FPerf_OnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FPerf_OnQuestFailed OnQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FPerf_OnObjectiveCompleted OnObjectiveCompleted;

    // ── Methods ───────────────────────────────────────────────────────────────

    /** Activate a quest by ID. Returns false if quest not found or already active. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    /** Mark a specific objective as complete */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    /** Force-complete a quest (all objectives done) */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest(const FString& QuestID);

    /** Fail the active quest */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    /** Get quest data by ID */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool GetQuestData(const FString& QuestID, FPerf_QuestData& OutData) const;

    /** Check if a quest is complete */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestComplete(const FString& QuestID) const;

    /** Initialize the tutorial quest "Find Water" */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Tutorial")
    void InitTutorialQuest();

    /** Called each tick to check proximity objectives */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckProximityObjectives(const FVector& PlayerLocation);

private:
    /** Internal: check if all objectives in a quest are complete */
    bool AreAllObjectivesComplete(const FPerf_QuestData& Quest) const;

    /** Internal: find quest index by ID */
    int32 FindQuestIndex(const FString& QuestID) const;

    float ElapsedTime = 0.0f;
};
