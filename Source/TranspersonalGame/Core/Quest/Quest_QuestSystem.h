#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Quest_QuestSystem.generated.h"

// Forward declarations
class UQuest_QuestObjective;
class AQuest_QuestGiver;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RewardExperience;

    FQuest_QuestData()
    {
        QuestID = TEXT("");
        QuestTitle = TEXT("");
        QuestDescription = TEXT("");
        QuestType = EQuestType::Hunt;
        Status = EQuestStatus::NotStarted;
        RequiredAmount = 1;
        CurrentProgress = 0;
        TargetLocation = FVector::ZeroVector;
        RewardExperience = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_QuestSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_QuestSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateQuestProgress(const FString& QuestID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetCompletedQuests();

    // Quest creation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntQuest(const FString& QuestID, const FString& TargetDinosaur, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatherQuest(const FString& QuestID, const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateExploreQuest(const FString& QuestID, const FVector& TargetLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateCraftQuest(const FString& QuestID, const FString& ItemToCraft, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateSurviveQuest(const FString& QuestID, float SurvivalTime);

    // Quest events
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnDinosaurKilled(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnResourceGathered(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnLocationReached(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnItemCrafted(const FString& ItemType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnSurvivalTimeReached(float TimeInSeconds);

protected:
    // Quest storage
    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FString> CompletedQuestIDs;

    // Quest templates
    void InitializeDefaultQuests();
    void CreateQuestTemplate(const FString& QuestID, const FString& Title, const FString& Description, 
                           EQuestType Type, int32 RequiredAmount, const FVector& Location, float Reward);

    // Internal quest processing
    void ProcessQuestCompletion(const FString& QuestID);
    bool IsQuestComplete(const FQuest_QuestData& QuestData);
    void NotifyQuestUpdate(const FString& QuestID);
};