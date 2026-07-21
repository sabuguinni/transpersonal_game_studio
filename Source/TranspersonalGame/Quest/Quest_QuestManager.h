#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Quest_QuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    FString Description;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    EQuestObjectiveType Type;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    int32 TargetCount;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    bool bCompleted;

    FQuest_QuestObjective()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        Type = EQuestObjectiveType::Kill;
        TargetCount = 1;
        CurrentCount = 0;
        bCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    FString Title;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    FString Description;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    EQuestStatus Status;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    TArray<FQuest_QuestObjective> Objectives;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    TArray<FString> ItemRewards;

    FQuest_QuestData()
    {
        QuestID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
        Status = EQuestStatus::NotStarted;
        ExperienceReward = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_QuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_QuestManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_QuestData GetQuestData(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_QuestData> GetActiveQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_QuestData> GetCompletedQuests();

    // Quest registration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_QuestData& QuestData);

    // Event handling
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnDinosaurKilled(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnItemCollected(const FString& ItemType);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnLocationReached(const FString& LocationName);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TMap<FString, FQuest_QuestData> RegisteredQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TMap<FString, FQuest_QuestData> ActiveQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TMap<FString, FQuest_QuestData> CompletedQuests;

private:
    void InitializeDefaultQuests();
    void CheckQuestCompletion(const FString& QuestID);
    void BroadcastQuestUpdate(const FString& QuestID);
};