#pragma once

// QuestObjectiveSystem.h
// Agent #15 — Narrative & Dialogue | PROD_CYCLE_AUTO_20260630_007
// Quest objective tracking, dialogue trigger actor, NPC interaction structs

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "QuestObjectiveSystem.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_ObjectiveStatus : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Skipped     UMETA(DisplayName = "Skipped")
};

UENUM(BlueprintType)
enum class ENarr_ObjectiveType : uint8
{
    Reach       UMETA(DisplayName = "Reach Location"),
    Collect     UMETA(DisplayName = "Collect Resource"),
    Survive     UMETA(DisplayName = "Survive Duration"),
    Eliminate   UMETA(DisplayName = "Eliminate Threat"),
    Escort      UMETA(DisplayName = "Escort NPC"),
    Craft       UMETA(DisplayName = "Craft Item"),
    Observe     UMETA(DisplayName = "Observe Event"),
    Interact    UMETA(DisplayName = "Interact with Object")
};

// ============================================================
// STRUCTS — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    FName ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    FText ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    ENarr_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    ENarr_ObjectiveStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    float ProximityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    bool bOptional;

    FNarr_ObjectiveData()
        : ObjectiveID(NAME_None)
        , ObjectiveType(ENarr_ObjectiveType::Reach)
        , Status(ENarr_ObjectiveStatus::Inactive)
        , RequiredCount(1)
        , CurrentCount(0)
        , TargetLocation(FVector::ZeroVector)
        , ProximityRadius(200.0f)
        , bOptional(false)
    {}
};

// ============================================================
// DELEGATES
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnQuestStarted, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnQuestCompleted, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnQuestFailed, FName, QuestID, FName, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnObjectiveCompleted, FName, QuestID, FName, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FNarr_OnObjectiveProgressCheck, FName, QuestID, FName, ObjectiveID, int32, Current, int32, Required);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNarr_OnDialogueTriggered, FName, SpeakerName, int32, LineIndex, FText, DialogueLine);

// ============================================================
// UNarr_QuestObjectiveComponent
// ============================================================

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNarr_QuestObjectiveComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_QuestObjectiveComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ---- Quest Control ----

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    bool StartQuest(FName QuestID, const TArray<FNarr_ObjectiveData>& Objectives);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    bool CompleteCurrentObjective();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    void CompleteQuest();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    bool FailQuest(FName Reason);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    void UpdateObjectiveCount(FName ObjectiveID, int32 Delta);

    // ---- State Queries ----

    UFUNCTION(BlueprintPure, Category = "Narrative|Quest")
    FNarr_ObjectiveData GetCurrentObjective() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Quest")
    TArray<FNarr_ObjectiveData> GetAllObjectives() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Quest")
    bool IsQuestActive() const { return bQuestActive; }

    UFUNCTION(BlueprintPure, Category = "Narrative|Quest")
    FName GetActiveQuestID() const { return ActiveQuestID; }

    UFUNCTION(BlueprintPure, Category = "Narrative|Quest")
    int32 GetCurrentObjectiveIndex() const { return CurrentObjectiveIndex; }

    // ---- Delegates ----

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Quest|Events")
    FNarr_OnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Quest|Events")
    FNarr_OnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Quest|Events")
    FNarr_OnQuestFailed OnQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Quest|Events")
    FNarr_OnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Quest|Events")
    FNarr_OnObjectiveProgressCheck OnObjectiveProgressCheck;

private:
    UPROPERTY()
    FName ActiveQuestID;

    UPROPERTY()
    TArray<FNarr_ObjectiveData> ActiveObjectives;

    UPROPERTY()
    bool bQuestActive;

    UPROPERTY()
    int32 CurrentObjectiveIndex;

    UPROPERTY()
    int32 TotalObjectivesCompleted;

    FTimerHandle ObjectiveCheckTimer;
    bool unregisterTimer;

    void CheckObjectiveConditions();
};

// ============================================================
// ANarr_DialogueTriggerActor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTriggerActor();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

public:
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void ResetTrigger();

    // ---- Config ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FText> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 DialogueLineIndex;

    // ---- State ----

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bDialogueTriggered;

    // ---- Events ----

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Dialogue|Events")
    FNarr_OnDialogueTriggered OnDialogueTriggered;

private:
    UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;
};
