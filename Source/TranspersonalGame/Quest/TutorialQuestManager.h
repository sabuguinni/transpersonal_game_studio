// TutorialQuestManager.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260624_003
// Tutorial quest system: Find Water + Find Food objectives tied to SurvivalComponent delegates

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TutorialQuestManager.generated.h"

// ─── Quest state enum ───────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_QuestState : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

// ─── Quest data struct ───────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector ObjectiveLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ObjectiveRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EWorld_QuestState State;

    FWorld_QuestData()
        : QuestID(NAME_None)
        , ObjectiveLocation(FVector::ZeroVector)
        , ObjectiveRadius(300.0f)
        , State(EWorld_QuestState::Inactive)
    {}
};

// ─── Delegates ───────────────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorld_QuestStarted,   FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorld_QuestCompleted, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorld_QuestFailed,    FName, QuestID);

// ─── Component ───────────────────────────────────────────────────────────────
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Tutorial Quest Manager")
class TRANSPERSONALGAME_API UTutorialQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UTutorialQuestManager();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Quest control ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(FName QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EWorld_QuestState GetQuestState(FName QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FWorld_QuestData GetQuestData(FName QuestID) const;

    // ── Survival integration ───────────────────────────────────────────────
    UFUNCTION()
    void OnThirstLow();

    UFUNCTION()
    void OnHungerLow();

    // ── Delegates ─────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnWorld_QuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnWorld_QuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnWorld_QuestFailed OnQuestFailed;

    // ── Config ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float ProximityCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    bool bAutoStartTutorial;

private:
    UPROPERTY()
    TMap<FName, FWorld_QuestData> QuestRegistry;

    float TimeSinceLastProximityCheck;

    void RegisterTutorialQuests();
    void CheckProximityObjectives();
    void BindSurvivalDelegates();
};
