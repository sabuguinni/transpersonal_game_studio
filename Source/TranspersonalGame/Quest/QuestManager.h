#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestManager.generated.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestManager.h — Quest system for dinosaur survival game
// All types prefixed Quest_ per RULE 2 (unique names)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    HuntCreature    UMETA(DisplayName = "Hunt Creature"),
    GatherResource  UMETA(DisplayName = "Gather Resource"),
    SurviveDuration UMETA(DisplayName = "Survive Duration"),
    DefendLocation  UMETA(DisplayName = "Defend Location"),
    EscapeArea      UMETA(DisplayName = "Escape Area")
};

UENUM(BlueprintType)
enum class EQuest_Priority : uint8
{
    Main        UMETA(DisplayName = "Main Quest"),
    Side        UMETA(DisplayName = "Side Quest"),
    Survival    UMETA(DisplayName = "Survival Challenge"),
    Discovery   UMETA(DisplayName = "Discovery")
};

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
        , TargetLocation(FVector::ZeroVector)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverActorLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bAutoActivate;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , Description(TEXT(""))
        , State(EQuest_State::Inactive)
        , Priority(EQuest_Priority::Main)
        , GiverActorLabel(TEXT(""))
        , bAutoActivate(false)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestActivated, FString, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FString, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FString, QuestID, FString, ObjectiveID);

/**
 * UQuestManager — World Subsystem managing all quests and objectives.
 * Handles quest state, objective tracking, and quest progression
 * for the dinosaur survival game.
 */
UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Quest Lifecycle ──────────────────────────────────────────────

    /** Register a new quest definition */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    /** Activate a registered quest by ID */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    /** Complete a quest (all objectives done) */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    /** Fail a quest */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    // ── Objective Tracking ───────────────────────────────────────────

    /** Advance an objective's progress */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    /** Check if all objectives of a quest are complete */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AreAllObjectivesComplete(const FString& QuestID) const;

    // ── Query ────────────────────────────────────────────────────────

    /** Get current state of a quest */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    /** Get all active quest IDs */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FString> GetActiveQuestIDs() const;

    /** Get all completed quest IDs */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FString> GetCompletedQuestIDs() const;

    /** Get quest data by ID */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool GetQuestData(const FString& QuestID, FQuest_Data& OutData) const;

    /** Get active quest count */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // ── Delegates ────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestActivated OnQuestActivated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

private:
    /** All registered quests keyed by QuestID */
    UPROPERTY()
    TMap<FString, FQuest_Data> QuestRegistry;

    /** Register the built-in starter quests */
    void RegisterStarterQuests();
};
