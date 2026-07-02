// HerdDisturbanceQuest.h — Agent #14 Quest & Mission Designer
// Quest: "Herd Disturbance" — player approaches herbivore herd, triggers stampede objective
// Wires to CrowdSimulationManager::OnStampedeTriggered delegate

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HerdDisturbanceQuest.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_HerdPhase : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    WatchingHerd    UMETA(DisplayName = "Watching Herd"),
    StampedeActive  UMETA(DisplayName = "Stampede Active"),
    EscapeWindow    UMETA(DisplayName = "Escape Window"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_HerdObjective : uint8
{
    ApproachHerd        UMETA(DisplayName = "Approach the Herd"),
    SurviveStampede     UMETA(DisplayName = "Survive the Stampede"),
    CollectResources    UMETA(DisplayName = "Collect Dropped Resources"),
    ReturnToElder       UMETA(DisplayName = "Return to Elder")
};

// ─── Structs (global scope — RULE 1) ─────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_HerdObjectiveState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Herd")
    EQuest_HerdObjective Objective = EQuest_HerdObjective::ApproachHerd;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Herd")
    bool bCompleted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Herd")
    FString Description = TEXT("Approach the herbivore herd within 800 meters.");
};

USTRUCT(BlueprintType)
struct FQuest_HerdReward
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Herd")
    int32 BoneCount = 3;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Herd")
    int32 HideCount = 2;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Herd")
    int32 MeatCount = 5;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Herd")
    FString RewardDescription = TEXT("Bones, hide, and meat from the stampede aftermath.");
};

// ─── Delegates ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnHerdPhaseChanged, EQuest_HerdPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnObjectiveCompleted, EQuest_HerdObjective, CompletedObjective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FQuest_OnHerdQuestCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FQuest_OnHerdQuestFailed);

// ─── Main Quest Actor ─────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Quest", BlueprintType, Blueprintable, meta = (DisplayName = "Herd Disturbance Quest"))
class TRANSPERSONALGAME_API AQuest_HerdDisturbanceManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_HerdDisturbanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Components ────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* ApproachTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* StampedeKillZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* QuestMarkerMesh;

    // ── Quest State ───────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadWrite, Category = "Quest|State")
    EQuest_HerdPhase CurrentPhase;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|State")
    TArray<FQuest_HerdObjectiveState> Objectives;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|State")
    FQuest_HerdReward QuestReward;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|State")
    float StampedeTimeLimit;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|State")
    float StampedeElapsed;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|State")
    bool bPlayerInKillZone;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|State")
    int32 ResourcesCollected;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|State")
    int32 ResourcesRequired;

    // ── Configuration ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float ApproachRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float KillZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float StampedeDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    FVector HerdLocation;

    // ── Delegates ─────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnHerdPhaseChanged OnHerdPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnHerdQuestCompleted OnHerdQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnHerdQuestFailed OnHerdQuestFailed;

    // ── Functions ─────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Herd")
    void ActivateQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest|Herd")
    void SetPhase(EQuest_HerdPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Quest|Herd")
    void CompleteObjective(EQuest_HerdObjective Objective);

    UFUNCTION(BlueprintCallable, Category = "Quest|Herd")
    void OnStampedeTriggeredCallback(FVector TriggerLocation, int32 AffectedAgents);

    UFUNCTION(BlueprintCallable, Category = "Quest|Herd")
    void OnResourceCollected();

    UFUNCTION(BlueprintCallable, Category = "Quest|Herd")
    void CheckQuestCompletion();

    UFUNCTION(BlueprintCallable, Category = "Quest|Herd")
    bool IsObjectiveComplete(EQuest_HerdObjective Objective) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Herd")
    FString GetCurrentObjectiveText() const;

    UFUNCTION(BlueprintPure, Category = "Quest|Herd")
    EQuest_HerdPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Quest|Herd")
    float GetStampedeProgress() const;

    UFUNCTION(CallInEditor, Category = "Quest|Debug")
    void DebugLogQuestState() const;

private:
    void InitializeObjectives();
    void TickStampede(float DeltaTime);

    UFUNCTION()
    void OnApproachOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnKillZoneOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnKillZoneOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

// ─── Quest NPC Elder Actor ────────────────────────────────────────────────────

UCLASS(ClassGroup = "Quest", BlueprintType, Blueprintable, meta = (DisplayName = "Tribal Elder NPC"))
class TRANSPERSONALGAME_API AQuest_TribalElderNPC : public AActor
{
    GENERATED_BODY()

public:
    AQuest_TribalElderNPC();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
    UStaticMeshComponent* ElderMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
    USphereComponent* InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Quest")
    TWeakObjectPtr<AQuest_HerdDisturbanceManager> LinkedQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    FString GreetingLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    FString QuestOfferLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    FString QuestCompleteLine;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    bool bQuestOffered;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    bool bQuestComplete;

    UFUNCTION(BlueprintCallable, Category = "NPC|Interaction")
    void InteractWithPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC|Interaction")
    FString GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Quest")
    void OnQuestCompleted();

private:
    UFUNCTION()
    void OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
