#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/TriggerVolume.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "Quest_QuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentObjectiveIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RewardExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> RewardItems;

    FQuest_QuestData()
    {
        QuestID = "";
        QuestName = "";
        Description = "";
        Status = EQuest_QuestStatus::NotStarted;
        CurrentObjectiveIndex = 0;
        RewardExperience = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bCompleted;

    FQuest_ObjectiveData()
    {
        ObjectiveID = "";
        Description = "";
        Type = EQuest_ObjectiveType::Kill;
        TargetID = "";
        RequiredCount = 1;
        CurrentCount = 0;
        bCompleted = false;
    }
};

/**
 * Quest Manager Subsystem
 * Handles quest progression, objective tracking, and NPC interactions
 */
UCLASS(BlueprintType)
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

    // Objective management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    // Quest data access
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    // NPC interaction
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuestNPC(APawn* NPC, const FString& NPCID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FString> GetAvailableQuestsForNPC(const FString& NPCID) const;

    // Trigger interaction
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuestTrigger(ATriggerVolume* Trigger, const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnTriggerEntered(const FString& QuestID, APawn* Player);

protected:
    // Quest data storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TMap<FString, FQuest_QuestData> QuestDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TMap<FString, FQuest_ObjectiveData> ObjectiveDatabase;

    // Active quest tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> CompletedQuests;

    // NPC and trigger mappings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TMap<FString, APawn*> QuestNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TMap<FString, ATriggerVolume*> QuestTriggers;

    // Quest initialization
    void InitializeQuestDatabase();
    void CreateSurvivalQuests();
    void CreateHuntingQuests();
    void CreateExplorationQuests();

    // Quest validation
    bool ValidateQuestData(const FQuest_QuestData& QuestData) const;
    bool ValidateObjectiveData(const FQuest_ObjectiveData& ObjectiveData) const;

    // Objective processing
    void ProcessKillObjective(const FString& QuestID, const FString& ObjectiveID, const FString& TargetType);
    void ProcessCollectObjective(const FString& QuestID, const FString& ObjectiveID, const FString& ItemType);
    void ProcessLocationObjective(const FString& QuestID, const FString& ObjectiveID, const FVector& Location);

    // Reward handling
    void GrantQuestRewards(const FQuest_QuestData& QuestData);
};