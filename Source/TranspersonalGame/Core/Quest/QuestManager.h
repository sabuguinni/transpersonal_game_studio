// QuestManager.h — Tutorial Quest System for Prehistoric Survival Game
// Agent #4 — Performance Optimizer | Cycle PROD_CYCLE_AUTO_20260627_005
// Manages quest states, objectives, and tutorial flow (Find Water, Hunt Prey, etc.)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

// ── ENUMS (global scope — UHT requirement) ────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Tutorial    UMETA(DisplayName = "Tutorial"),
    Survival    UMETA(DisplayName = "Survival"),
    Exploration UMETA(DisplayName = "Exploration"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Craft       UMETA(DisplayName = "Craft")
};

// ── STRUCTS (global scope — UHT requirement) ──────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CurrentValue;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , bCompleted(false)
        , TargetValue(1.0f)
        , CurrentValue(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;  // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestType(EQuest_Type::Tutorial)
        , QuestState(EQuest_State::Inactive)
        , TimeLimit(0.0f)
        , ElapsedTime(0.0f)
    {}
};

// ── QUEST MANAGER ACTOR ───────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Survival")
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Active Quest Registry ─────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Data> ActiveQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Data> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bAutoStartTutorial;

    // ── Quest Management ──────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(const FString& QuestID, const FString& ObjectiveID, float Value = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    FString GetActiveQuestObjectiveText() const;

    // ── Tutorial Quest Triggers ───────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest|Tutorial")
    void OnPlayerReachedWaterSource();

    UFUNCTION(BlueprintCallable, Category = "Quest|Tutorial")
    void OnPlayerDrank(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest|Tutorial")
    void OnPlayerHungerLow(float HungerValue);

    UFUNCTION(BlueprintCallable, Category = "Quest|Tutorial")
    void OnPlayerCraftedTool();

    // ── Survival Stat Hooks (called by SurvivalComponent) ────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void OnThirstChanged(float NewThirst);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void OnHungerChanged(float NewHunger);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void OnPlayerTookDamage(float DamageAmount);

private:
    // ── Internal Quest Builders ───────────────────────────────────────────
    FQuest_Data BuildTutorialFindWater() const;
    FQuest_Data BuildTutorialFindFood() const;
    FQuest_Data BuildTutorialCraftTool() const;
    FQuest_Data BuildTutorialFindShelter() const;

    void CheckQuestCompletion(FQuest_Data& Quest);
    void OnQuestCompleted(const FQuest_Data& Quest);
    int32 FindQuestIndex(const TArray<FQuest_Data>& QuestList, const FString& QuestID) const;

    // ── Tutorial sequence tracking ────────────────────────────────────────
    int32 TutorialStepIndex;
    static const TArray<FString> TutorialSequence;
};
