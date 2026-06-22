// QuestManager.h
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260622_005

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestManager.generated.h"

// ── Enums (global scope — RULE 1) ──────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Defend      UMETA(DisplayName = "Defend"),
    Track       UMETA(DisplayName = "Track"),
    Craft       UMETA(DisplayName = "Craft"),
    Explore     UMETA(DisplayName = "Explore"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Migrate     UMETA(DisplayName = "Migrate")
};

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    ProtectNPC      UMETA(DisplayName = "Protect NPC"),
    CraftItem       UMETA(DisplayName = "Craft Item"),
    ObserveEvent    UMETA(DisplayName = "Observe Event"),
    SurviveTime     UMETA(DisplayName = "Survive Time")
};

// ── Structs (global scope — RULE 1) ────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType = EQuest_ObjectiveType::KillTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LocationRadius = 300.0f;
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 FoodReward = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 MaterialReward = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedRecipe = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedArea = TEXT("");
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType = EQuest_Type::Hunt;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_Status Status = EQuest_Status::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName GiverNPCID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f;  // 0 = no time limit

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float ElapsedTime = 0.0f;
};

// ── QuestManager class ──────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuestManager : public UObject
{
    GENERATED_BODY()

public:
    UQuestManager();

    // Initialize with starter quests
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void InitializeQuestSystem();

    // Activate a quest by ID
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(FName QuestID);

    // Update objective progress
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjectiveProgress(FName QuestID, int32 ObjectiveIndex, int32 Amount);

    // Check if all objectives complete and mark quest done
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CheckQuestCompletion(FName QuestID);

    // Fail a quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(FName QuestID);

    // Get active quests
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    // Get quest by ID
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool GetQuestData(FName QuestID, FQuest_Data& OutData) const;

    // Tick for time-limited quests
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void TickQuestTimers(float DeltaTime);

    // Called when crowd herd flees (from CrowdSimulationManager)
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnHerdFleeTriggered(FVector FleeOrigin, float Radius);

    // Get total completed quest count
    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetCompletedQuestCount() const;

    // All registered quests
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Data> AllQuests;

private:
    void RegisterStarterQuests();
    FQuest_Data BuildHuntQuest();
    FQuest_Data BuildGatherQuest();
    FQuest_Data BuildDefendQuest();
    FQuest_Data BuildTrackHerdQuest();
    FQuest_Data BuildCraftAxeQuest();

    int32 FindQuestIndex(FName QuestID) const;
};
