// QuestManager.h
// Quest & Mission Designer — Agent #14
// Transpersonal Game Studio — Prehistoric Survival Game
// Manages quest state, objectives, and progression for the player.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS
// ─────────────────────────────────────────────────────────────────────────────

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
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    EscapeZone      UMETA(DisplayName = "Escape Zone"),
    FollowTarget    UMETA(DisplayName = "Follow Target"),
    SurviveDuration UMETA(DisplayName = "Survive Duration")
};

UENUM(BlueprintType)
enum class EQuest_QuestID : uint8
{
    None                    UMETA(DisplayName = "None"),
    Hunt_RaptorPack         UMETA(DisplayName = "Hunt: Raptor Pack"),
    Survive_TRexTerritory   UMETA(DisplayName = "Survive: T-Rex Territory"),
    Follow_BrachMigration   UMETA(DisplayName = "Follow: Brachiosaurus Migration"),
    Gather_Supplies         UMETA(DisplayName = "Gather: Camp Supplies"),
    Defend_Camp             UMETA(DisplayName = "Defend: Camp Attack")
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType = EQuest_ObjectiveType::ReachLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName TargetActorLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ProximityRadius = 300.0f;
};

USTRUCT(BlueprintType)
struct FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestID QuestID = EQuest_QuestID::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State State = EQuest_State::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ActiveObjectiveIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f;  // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// DELEGATES
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestStarted,    EQuest_QuestID, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestCompleted,  EQuest_QuestID, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestFailed,     EQuest_QuestID, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnObjectiveUpdated, EQuest_QuestID, QuestID, int32, ObjectiveIndex);

// ─────────────────────────────────────────────────────────────────────────────
// QUEST MANAGER COMPONENT
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Quest Manager")
class TRANSPERSONALGAME_API UQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManager();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Quest Control ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(EQuest_QuestID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteCurrentObjective();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(EQuest_QuestID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceObjective(EQuest_QuestID QuestID, int32 IncrementAmount = 1);

    // ── Query ──────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestActive(EQuest_QuestID QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(EQuest_QuestID QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    FQuest_QuestData GetQuestData(EQuest_QuestID QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    FQuest_Objective GetActiveObjective(EQuest_QuestID QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    EQuest_QuestID GetCurrentActiveQuest() const { return CurrentActiveQuestID; }

    // ── Trigger Integration (from Agent #13 crowd delegates) ───────────────
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnteredTRexTerritory();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnteredRaptorZone();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnteredBrachMigrationCorridor();

    // ── Delegates ─────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnQuestFailed OnQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnObjectiveUpdated OnObjectiveUpdated;

    // ── State ──────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_QuestData> QuestRegistry;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    EQuest_QuestID CurrentActiveQuestID = EQuest_QuestID::None;

private:
    void InitializeQuestRegistry();
    FQuest_QuestData BuildRaptorPackQuest() const;
    FQuest_QuestData BuildTRexTerritoryQuest() const;
    FQuest_QuestData BuildBrachMigrationQuest() const;
    FQuest_QuestData BuildGatherSuppliesQuest() const;
    FQuest_QuestData BuildDefendCampQuest() const;

    void TickActiveQuestTimers(float DeltaTime);
    void CheckProximityObjectives();

    FQuest_QuestData* FindQuestByID(EQuest_QuestID QuestID);
    const FQuest_QuestData* FindQuestByIDConst(EQuest_QuestID QuestID) const;
};
