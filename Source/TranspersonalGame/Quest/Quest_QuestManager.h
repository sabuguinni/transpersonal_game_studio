#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Quest_QuestManager.generated.h"

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
    EQuestStatus QuestStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ObjectiveTexts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<bool> ObjectiveCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RewardExperience;

    FQuest_QuestData()
    {
        QuestID = "";
        QuestTitle = "";
        QuestDescription = "";
        QuestType = EQuestType::Hunt;
        QuestStatus = EQuestStatus::NotStarted;
        TargetCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        RewardExperience = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetTag;

    FQuest_ObjectiveData()
    {
        ObjectiveID = "";
        ObjectiveText = "";
        ObjectiveType = EObjectiveType::Kill;
        bIsCompleted = false;
        RequiredCount = 1;
        CurrentProgress = 0;
        TargetTag = "";
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_QuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_QuestManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_QuestData> GetCompletedQuests() const;

    // Objective management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const;

    // Quest creation
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateHuntQuest(const FString& QuestID, const FString& Title, const FString& TargetSpecies, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateExploreQuest(const FString& QuestID, const FString& Title, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateGatherQuest(const FString& QuestID, const FString& Title, const FString& ResourceType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateSurvivalQuest(const FString& QuestID, const FString& Title, float Duration);

    // Event handlers
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnDinosaurKilled(const FString& Species);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnLocationReached(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnResourceGathered(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerSurvived(float TimeAlive);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CompletedQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    bool bQuestSystemEnabled;

private:
    void InitializeDefaultQuests();
    void CheckQuestCompletion(const FString& QuestID);
    bool CanStartQuest(const FString& QuestID) const;
    void NotifyQuestUpdate(const FString& QuestID);
};