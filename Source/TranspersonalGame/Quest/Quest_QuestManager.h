#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_QuestManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_QuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    TurnedIn        UMETA(DisplayName = "Turned In")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Kill            UMETA(DisplayName = "Kill Target"),
    Collect         UMETA(DisplayName = "Collect Items"),
    Reach           UMETA(DisplayName = "Reach Location"),
    Interact        UMETA(DisplayName = "Interact With Object"),
    Survive         UMETA(DisplayName = "Survive Duration"),
    Craft           UMETA(DisplayName = "Craft Items")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    FQuest_Objective()
    {
        Description = TEXT("");
        Type = EQuest_ObjectiveType::Collect;
        TargetName = TEXT("");
        RequiredAmount = 1;
        CurrentAmount = 0;
        bCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float StartTime;

    FQuest_QuestData()
    {
        QuestID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
        Status = EQuest_QuestStatus::NotStarted;
        QuestGiverName = TEXT("");
        ExperienceReward = 0;
        TimeLimit = 0.0f;
        StartTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_QuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_QuestManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool TurnInQuest(const FString& QuestID);

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjectiveProgress(const FString& QuestID, int32 ObjectiveIndex, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(const FString& QuestID, int32 ObjectiveIndex);

    // Quest Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_QuestData> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    // Quest Registration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void InitializeDefaultQuests();

    // Event Handlers
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerKilledTarget(const FString& TargetName);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerCollectedItem(const FString& ItemName, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerReachedLocation(const FString& LocationName);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerInteractedWithObject(const FString& ObjectName);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerCraftedItem(const FString& ItemName, int32 Amount);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FString> CompletedQuestIDs;

    void CheckQuestObjectives(const FString& QuestID);
    void GiveQuestRewards(const FQuest_QuestData& QuestData);
    void BroadcastQuestUpdate(const FString& QuestID, const FString& UpdateMessage);
};