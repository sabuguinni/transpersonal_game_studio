#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalQuestManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_QuestType : uint8
{
    Hunt           UMETA(DisplayName = "Hunt Dinosaur"),
    Gather         UMETA(DisplayName = "Gather Resources"),
    Survive        UMETA(DisplayName = "Survival Challenge"),
    Explore        UMETA(DisplayName = "Explore Territory"),
    Craft          UMETA(DisplayName = "Craft Tools"),
    Defend         UMETA(DisplayName = "Defend Camp")
};

UENUM(BlueprintType)
enum class EQuest_QuestStatus : uint8
{
    NotStarted     UMETA(DisplayName = "Not Started"),
    Active         UMETA(DisplayName = "Active"),
    Completed      UMETA(DisplayName = "Completed"),
    Failed         UMETA(DisplayName = "Failed"),
    Abandoned      UMETA(DisplayName = "Abandoned")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_QuestObjective()
    {
        Description = TEXT("");
        TargetCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_QuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    FQuest_SurvivalQuest()
    {
        QuestID = TEXT("");
        QuestName = TEXT("");
        QuestDescription = TEXT("");
        QuestType = EQuest_QuestType::Gather;
        Status = EQuest_QuestStatus::NotStarted;
        ExperienceReward = 100;
        TimeLimit = 300.0f;
        bHasTimeLimit = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SurvivalQuestManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjective(const FString& QuestID, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalQuest GetQuest(const FString& QuestID) const;

    // Quest creation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntQuest(const FString& TargetDinosaur, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatherQuest(const FString& ResourceType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateSurvivalQuest(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateExploreQuest(const FString& LocationName);

    // Event handlers
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnDinosaurKilled(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnResourceGathered(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnLocationDiscovered(const FString& LocationName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnPlayerSurvived(float Duration);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> AllQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 NextQuestID;

    // Helper functions
    void InitializeDefaultQuests();
    FString GenerateQuestID();
    void CheckQuestCompletion(FQuest_SurvivalQuest& Quest);
    void BroadcastQuestUpdate(const FQuest_SurvivalQuest& Quest);

private:
    FTimerHandle QuestTimerHandle;
    void UpdateQuestTimers();
};