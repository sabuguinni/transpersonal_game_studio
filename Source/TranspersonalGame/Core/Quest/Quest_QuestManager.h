#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Quest_QuestManager.generated.h"

class UQuest_QuestObjective;
class UQuest_QuestInstance;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FString QuestName;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FString> ObjectiveIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 RequiredLevel;

    FQuest_QuestData()
    {
        QuestID = TEXT("");
        QuestName = TEXT("");
        QuestDescription = TEXT("");
        bIsMainQuest = false;
        RequiredLevel = 1;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UQuest_QuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    TArray<FQuest_QuestData> GetCompletedQuests() const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    bool UpdateObjectiveProgress(const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    bool CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    int32 GetObjectiveProgress(const FString& ObjectiveID) const;

    // Quest Registration
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void RegisterQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void RegisterSurvivalQuests();

protected:
    UPROPERTY()
    TMap<FString, FQuest_QuestData> RegisteredQuests;

    UPROPERTY()
    TArray<FString> ActiveQuestIDs;

    UPROPERTY()
    TArray<FString> CompletedQuestIDs;

    UPROPERTY()
    TMap<FString, int32> ObjectiveProgress;

private:
    void InitializeSurvivalQuests();
    void CheckQuestCompletion(const FString& QuestID);
    bool AreAllObjectivesComplete(const FString& QuestID) const;
};