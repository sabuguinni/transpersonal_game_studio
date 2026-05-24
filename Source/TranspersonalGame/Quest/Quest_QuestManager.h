#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Quest_QuestManager.generated.h"

class ATranspersonalCharacter;
class UQuest_QuestComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    ESurvivalResourceType RequiredResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RewardExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        RequiredResourceType = ESurvivalResourceType::Wood;
        RequiredAmount = 1;
        RewardExperience = 10.0f;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<ESurvivalResourceType> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    FQuest_QuestData()
    {
        QuestID = TEXT("");
        QuestName = TEXT("");
        QuestDescription = TEXT("");
        RequiredLevel = 1;
        ExperienceReward = 100.0f;
        bIsMainQuest = false;
        bIsRepeatable = false;
    }
};

UENUM(BlueprintType)
enum class EQuest_QuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ActiveQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_QuestData QuestData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<bool> ObjectiveCompletionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float StartTime;

    FQuest_ActiveQuest()
    {
        QuestID = TEXT("");
        Status = EQuest_QuestStatus::NotStarted;
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
    bool StartQuest(const FString& QuestID, ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID, ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(const FString& QuestID, ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateQuestProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    // Quest Queries
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_ActiveQuest> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_ActiveQuest GetQuestByID(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    // Survival Integration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnResourceCollected(ESurvivalResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnDinosaurKilled(EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnAreaExplored(const FString& AreaName);

    // Data Management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void LoadQuestDatabase();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_QuestData> GetAvailableQuests(int32 PlayerLevel) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    UDataTable* QuestDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_ActiveQuest> ActiveQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bAutoCompleteObjectives;

private:
    void InitializeDefaultQuests();
    bool CheckQuestCompletion(FQuest_ActiveQuest& Quest);
    void GiveQuestRewards(const FQuest_QuestData& QuestData, ATranspersonalCharacter* Player);
    int32 FindActiveQuestIndex(const FString& QuestID) const;
};