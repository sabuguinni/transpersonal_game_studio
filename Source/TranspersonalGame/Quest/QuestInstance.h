#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestManager.h"
#include "QuestInstance.generated.h"

class UQuestObjective;

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuestInstance : public UObject
{
    GENERATED_BODY()

public:
    UQuestInstance();

    // Initialization
    void Initialize(const FQuestData& QuestData);

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateQuest(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AreAllObjectivesCompleted() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool HasObjective(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void GiveRewards();

    // Getters
    UFUNCTION(BlueprintCallable, Category = "Quest")
    FString GetQuestID() const { return QuestData.QuestID; }

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FText GetQuestTitle() const { return QuestData.QuestTitle; }

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FText GetQuestDescription() const { return QuestData.QuestDescription; }

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuestType GetQuestType() const { return QuestData.QuestType; }

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<UQuestObjective*> GetObjectives() const { return Objectives; }

    UFUNCTION(BlueprintCallable, Category = "Quest")
    float GetCompletionPercentage() const;

    // Quest Progress
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetQuestProgress(float Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    float GetQuestProgress() const { return QuestProgress; }

    // Timer Management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuestTimer(float TimeLimit);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    float GetRemainingTime() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsTimedQuest() const { return bIsTimedQuest; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FQuestData QuestData;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<UQuestObjective*> Objectives;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float QuestProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bIsTimedQuest;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float TimeLimit;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float ElapsedTime;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FDateTime StartTime;

private:
    void CreateObjectives();
    void UpdateQuestProgress();
    void CheckTimeLimit(float DeltaTime);
};