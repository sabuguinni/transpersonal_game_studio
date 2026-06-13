#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Quest_ObjectiveSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    KillTarget,
    CollectItem,
    ReachLocation,
    TalkToNPC,
    Survive,
    Craft
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CompletionRadius;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        Type = EQuest_ObjectiveType::KillTarget;
        TargetID = TEXT("");
        RequiredCount = 1;
        CurrentCount = 0;
        bCompleted = false;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NPCGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExperienceReward;

    FQuest_QuestData()
    {
        QuestID = TEXT("");
        QuestName = TEXT("");
        Description = TEXT("");
        bActive = false;
        bCompleted = false;
        NPCGiver = TEXT("");
        ExperienceReward = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestCompleted, const FString&, QuestID);

UCLASS()
class TRANSPERSONALGAME_API UQuest_ObjectiveSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckLocationObjectives(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnDinosaurKilled(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnItemCollected(const FString& ItemType, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnNPCTalkedTo(const FString& NPCID);

    UPROPERTY(BlueprintAssignable)
    FQuest_OnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable)
    FQuest_OnQuestCompleted OnQuestCompleted;

protected:
    UPROPERTY()
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY()
    TArray<FString> ActiveQuestIDs;

    void InitializeDefaultQuests();
    void CheckObjectiveCompletion(FQuest_QuestData& Quest, FQuest_ObjectiveData& Objective);
    void CheckQuestCompletion(FQuest_QuestData& Quest);
};