// QuestManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260622_011
// Manages quest state, objectives, and NPC quest givers for the prehistoric survival game.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestManager.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Locked      UMETA(DisplayName = "Locked"),
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Defend      UMETA(DisplayName = "Defend"),
    Migrate     UMETA(DisplayName = "Migrate"),
    Gather      UMETA(DisplayName = "Gather"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Explore     UMETA(DisplayName = "Explore"),
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    EscortNPC       UMETA(DisplayName = "Escort NPC"),
    DefendArea      UMETA(DisplayName = "Defend Area"),
    SurviveTime     UMETA(DisplayName = "Survive Time"),
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FVector WorldLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    float RadiusTolerance;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    FQuest_Objective()
        : ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , WorldLocation(FVector::ZeroVector)
        , RadiusTolerance(200.0f)
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 BoneTokens;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    float ReputationGain;

    FQuest_Reward()
        : BoneTokens(0)
        , ReputationGain(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString GiverNPCLabel;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    FQuest_Data()
        : QuestType(EQuest_Type::Hunt)
        , QuestState(EQuest_State::Locked)
        , bIsMainQuest(false)
        , TimeLimit(0.0f)
    {}
};

// ─── QuestManager Subsystem ───────────────────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest registration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    // Objective tracking
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsObjectiveComplete(const FString& QuestID, const FString& ObjectiveID) const;

    // Query
    UFUNCTION(BlueprintPure, Category = "Quest")
    bool GetQuestData(const FString& QuestID, FQuest_Data& OutData) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetAvailableQuests() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    int32 GetTotalQuestCount() const;

    // NPC interaction
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FString> GetQuestsFromNPC(const FString& NPCLabel) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnterQuestRadius(const FString& QuestID, const FString& ObjectiveID);

    // Crowd integration — called by CrowdSimulationManager
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPredatorEnteredCampRadius(FVector PredatorLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnTribeReachedWaypoint(int32 WaypointIndex);

private:
    UPROPERTY()
    TMap<FString, FQuest_Data> QuestRegistry;

    void InitializeDefaultQuests();
    void CheckAllObjectivesComplete(const FString& QuestID);
    void GrantReward(const FQuest_Reward& Reward);
};
