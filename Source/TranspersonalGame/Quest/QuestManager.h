// QuestManager.h — Agent #14 Quest & Mission Designer
// Quest management subsystem for the prehistoric survival game.
// Handles quest state, objectives, rewards, and progression chains.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Defend      UMETA(DisplayName = "Defend"),
    Explore     UMETA(DisplayName = "Explore"),
    Craft       UMETA(DisplayName = "Craft"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Migrate     UMETA(DisplayName = "Migrate")
};

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Locked      UMETA(DisplayName = "Locked"),
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Kill        UMETA(DisplayName = "Kill"),
    Collect     UMETA(DisplayName = "Collect"),
    Craft       UMETA(DisplayName = "Craft"),
    Reach       UMETA(DisplayName = "Reach Location"),
    Survive     UMETA(DisplayName = "Survive"),
    Interact    UMETA(DisplayName = "Interact"),
    Escort      UMETA(DisplayName = "Escort")
};

// ============================================================
// STRUCTS — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FName ObjectiveID;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 TargetCount;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_Objective()
        : ObjectiveID(NAME_None)
        , ObjectiveType(EQuest_ObjectiveType::Kill)
        , TargetCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FName QuestID;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 RewardBones;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 RewardHide;

    FQuest_Data()
        : QuestID(NAME_None)
        , QuestType(EQuest_Type::Hunt)
        , QuestState(EQuest_State::Locked)
        , bIsMainQuest(false)
        , RewardBones(0)
        , RewardHide(0)
    {}
};

// ============================================================
// DELEGATES
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStarted,    FName, QuestID, FText, QuestTitle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnQuestCompleted, FName, QuestID, int32, RewardBones, int32, RewardHide);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, FName, QuestID, FName, ObjectiveID);

// ============================================================
// QUEST MANAGER — GameInstance Subsystem
// ============================================================

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ---- Quest Control ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool UpdateObjectiveProgress(FName QuestID, FName ObjectiveID, int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(FName QuestID);

    // ---- Query ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(FName QuestID) const;

    // ---- Delegates ----

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    // ---- Stats ----

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 ActiveQuestCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CompletedQuestCount;

private:
    void InitializeQuestDatabase();
    void CheckQuestCompletion(FName QuestID);
    void UnlockFollowUpQuests(FName CompletedQuestID);

    UPROPERTY()
    TMap<FName, FQuest_Data> QuestDatabase;

    UPROPERTY()
    TMap<FName, FQuest_Data> ActiveQuests;

    bool bQuestSystemInitialized;
};
