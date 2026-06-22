// QuestManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260622_006
// Prehistoric survival quest system — no spiritual/mystical content.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

// ── Enums (global scope — UHT requirement) ──────────────────────────────────

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
    TalkToNPC       UMETA(DisplayName = "Talk to NPC"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    DefendLocation  UMETA(DisplayName = "Defend Location"),
    CraftItem       UMETA(DisplayName = "Craft Item"),
    SurviveTime     UMETA(DisplayName = "Survive Time")
};

UENUM(BlueprintType)
enum class EQuest_RewardType : uint8
{
    CraftingRecipe  UMETA(DisplayName = "Crafting Recipe"),
    ResourceBundle  UMETA(DisplayName = "Resource Bundle"),
    TribeAlliance   UMETA(DisplayName = "Tribe Alliance"),
    MapReveal       UMETA(DisplayName = "Map Reveal"),
    WeaponBlueprint UMETA(DisplayName = "Weapon Blueprint")
};

// ── Structs (global scope — UHT requirement) ────────────────────────────────

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
    FString TargetActorLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , TargetActorLabel(TEXT(""))
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_RewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Quantity;

    FQuest_Reward()
        : RewardType(EQuest_RewardType::ResourceBundle)
        , RewardID(TEXT(""))
        , Quantity(1)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TriggerActorLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCLabel;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_State State;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , TriggerActorLabel(TEXT(""))
        , GiverNPCLabel(TEXT(""))
        , State(EQuest_State::Inactive)
    {}
};

// ── Quest Manager Actor ─────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Quest Registry ────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Definition> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|State")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|State")
    TArray<FString> CompletedQuestIDs;

    // ── Quest Operations ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Definition GetQuestDefinition(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetActiveQuestCount() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetCompletedQuestCount() const;

    // ── Trigger Integration ───────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnterTrigger(const FString& TriggerActorLabel);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnNPCInteraction(const FString& NPCLabel);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnDinosaurKilled(const FString& DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnItemCrafted(const FString& ItemID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnLocationReached(const FString& LocationLabel);

    // ── Crowd Integration ─────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Crowd")
    void TriggerCrowdFleeEvent(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crowd")
    void SetCrowdAgentQuestState(const FString& AgentLabel, bool bQuestActive);

private:
    void InitializeDefaultQuests();
    FQuest_Definition* FindQuest(const FString& QuestID);
    const FQuest_Definition* FindQuestConst(const FString& QuestID) const;
    void CheckAllObjectivesComplete(FQuest_Definition& Quest);
    void GrantReward(const FQuest_Reward& Reward);
};
