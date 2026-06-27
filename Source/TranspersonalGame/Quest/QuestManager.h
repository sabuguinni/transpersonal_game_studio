// QuestManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260627_005
// Manages all active quests, objectives, and quest state for the survival game.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Reach       UMETA(DisplayName = "Reach Location"),
    Kill        UMETA(DisplayName = "Kill Target"),
    Gather      UMETA(DisplayName = "Gather Resource"),
    Survive     UMETA(DisplayName = "Survive Duration"),
    Follow      UMETA(DisplayName = "Follow Target"),
    Defend      UMETA(DisplayName = "Defend Location")
};

UENUM(BlueprintType)
enum class EQuest_ID : uint8
{
    None                UMETA(DisplayName = "None"),
    Q1_TribeEncounter   UMETA(DisplayName = "Q1 Tribe Encounter"),
    Q2_RaptorAmbush     UMETA(DisplayName = "Q2 Raptor Ambush"),
    Q3_ForagingRun      UMETA(DisplayName = "Q3 Foraging Run"),
    Q4_FollowMigration  UMETA(DisplayName = "Q4 Follow Migration"),
    Q5_DefendCamp       UMETA(DisplayName = "Q5 Defend Camp")
};

// ─── Structs (global scope — RULE 1) ─────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::Reach)
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
        , TargetLocation(FVector::ZeroVector)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ID QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString NpcVoiceURL;

    FQuest_Data()
        : QuestID(EQuest_ID::None)
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , State(EQuest_State::Inactive)
        , TimeLimit(0.0f)
        , ElapsedTime(0.0f)
        , bHasTimeLimit(false)
        , NpcVoiceURL(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FQuest_RewardData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 BonusFood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 BonusWater;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedCraftingRecipe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedArea;

    FQuest_RewardData()
        : BonusFood(0)
        , BonusWater(0)
        , UnlockedCraftingRecipe(TEXT(""))
        , UnlockedArea(TEXT(""))
    {}
};

// ─── UQuestManager ────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest lifecycle
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest(EQuest_ID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest(EQuest_ID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(EQuest_ID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceObjective(EQuest_ID QuestID, const FString& ObjectiveID, int32 Amount = 1);

    // Query
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    EQuest_State GetQuestState(EQuest_ID QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(EQuest_ID QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_Data GetQuestData(EQuest_ID QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetAllActiveQuests() const;

    // Crowd integration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnteredQuestZone(const FString& ZoneLabel);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnDinoKilled(const FString& DinoType);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnResourceGathered(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnSurvivalTimerTick(float DeltaTime);

    // Rewards
    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_RewardData GetQuestReward(EQuest_ID QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void GrantReward(EQuest_ID QuestID);

    // Debug
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Debug")
    void PrintAllQuestStates() const;

private:
    void InitializeQuestDefinitions();
    FQuest_Data BuildQ1_TribeEncounter() const;
    FQuest_Data BuildQ2_RaptorAmbush() const;
    FQuest_Data BuildQ3_ForagingRun() const;
    FQuest_Data BuildQ4_FollowMigration() const;
    FQuest_Data BuildQ5_DefendCamp() const;

    UPROPERTY()
    TMap<EQuest_ID, FQuest_Data> QuestRegistry;

    UPROPERTY()
    TArray<EQuest_ID> ActiveQuestIDs;
};
