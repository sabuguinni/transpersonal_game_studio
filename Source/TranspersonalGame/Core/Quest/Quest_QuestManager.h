#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Quest_QuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    TArray<FString> ObjectiveIDs;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    bool bIsActive;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
    float QuestProgress;

    FQuest_QuestData()
    {
        QuestID = TEXT("");
        QuestTitle = TEXT("");
        QuestDescription = TEXT("");
        bIsActive = false;
        bIsCompleted = false;
        QuestProgress = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective")
    FString ObjectiveText;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective")
    EQuestObjectiveType ObjectiveType;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective")
    FString TargetActorClass;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Objective")
    bool bIsCompleted;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        ObjectiveText = TEXT("");
        ObjectiveType = EQuestObjectiveType::Kill;
        TargetActorClass = TEXT("");
        RequiredCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_QuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_QuestManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest System")
    TArray<FQuest_QuestData> ActiveQuests;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest System")
    TArray<FQuest_QuestData> CompletedQuests;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest System")
    TMap<FString, FQuest_ObjectiveData> QuestObjectives;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest System")
    int32 MaxActiveQuests;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Quest Management Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjective(const FString& ObjectiveID, int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID);

    // Objective Management Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AddObjectiveToQuest(const FString& QuestID, const FQuest_ObjectiveData& ObjectiveData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_ObjectiveData GetObjectiveData(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& ObjectiveID);

    // Crowd Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnCrowdEventTriggered(const FString& EventType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterCrowdQuestTrigger(const FString& QuestID, const FString& EventType);

private:
    void InitializeDefaultQuests();
    void CheckQuestCompletion(const FString& QuestID);
    void UpdateQuestProgress(const FString& QuestID);

    UPROPERTY()
    TMap<FString, FString> CrowdEventToQuestMap;
};