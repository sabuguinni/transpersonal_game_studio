#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "QuestSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_ObjectiveData()
    {
        ObjectiveDescription = TEXT("Default Objective");
        ObjectiveType = EQuest_ObjectiveType::Hunt;
        TargetCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status QuestStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    FQuest_Data()
    {
        QuestID = TEXT("QUEST_001");
        QuestName = TEXT("First Hunt");
        QuestDescription = TEXT("Hunt your first dinosaur to survive");
        QuestStatus = EQuest_Status::NotStarted;
        ExperienceReward = 100;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuestSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjective(const FString& QuestID, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuestGiver(AActor* QuestGiver, const FString& QuestID);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_Data> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_Data> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TMap<AActor*, FString> QuestGivers;

private:
    void InitializeDefaultQuests();
    void CheckObjectiveCompletion(FQuest_Data& Quest);
    void OnQuestCompleted(const FQuest_Data& Quest);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestComponent();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Quest Component")
    void TriggerQuestInteraction();

    UFUNCTION(BlueprintCallable, Category = "Quest Component")
    void SetQuestID(const FString& NewQuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Component")
    FString GetQuestID() const { return QuestID; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bAutoStartOnInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float InteractionRange;

private:
    UQuestSystem* QuestSystem;
};