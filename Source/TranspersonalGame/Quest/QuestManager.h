#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestManager.generated.h"

// ============================================================
// STRUCTS — must be at global scope (UE5 UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    FString ObjectiveID;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    bool bCompleted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    bool bOptional = false;
};

USTRUCT(BlueprintType)
struct FNarr_QuestData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    FString Title;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    TArray<FNarr_QuestObjective> Objectives;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    int32 RewardXP = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Quest")
    FString GiverNPCID;
};

USTRUCT(BlueprintType)
struct FNarr_ActiveQuest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Quest")
    FString QuestTitle;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Quest")
    int32 CurrentObjectiveIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Quest")
    bool bIsTracked = true;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Quest")
    TArray<FNarr_QuestObjective> Objectives;
};

// ============================================================
// DELEGATES
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnQuestStarted, const FString&, QuestID, const FString&, QuestTitle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnQuestCompleted, const FString&, QuestID, const FString&, QuestTitle);

// ============================================================
// QUEST MANAGER — GameInstance Subsystem
// ============================================================

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest lifecycle
    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    bool StartQuest(const FNarr_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    bool CompleteObjective(const FString& QuestID, int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    bool AbandonQuest(const FString& QuestID);

    // Queries
    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    FNarr_ActiveQuest GetActiveQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    TArray<FNarr_ActiveQuest> GetAllActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    int32 GetActiveQuestCount() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    int32 GetCompletedQuestCount() const;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Quest")
    FNarr_OnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Quest")
    FNarr_OnQuestCompleted OnQuestCompleted;

private:
    UPROPERTY()
    TArray<FNarr_ActiveQuest> ActiveQuests;

    UPROPERTY()
    TSet<FString> CompletedQuestIDs;

    UPROPERTY()
    int32 ActiveQuestCount;

    UPROPERTY()
    int32 CompletedQuestCount;

    UPROPERTY()
    bool bQuestSystemInitialized;
};
