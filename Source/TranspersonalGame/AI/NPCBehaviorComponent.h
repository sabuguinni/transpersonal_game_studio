#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Rest        UMETA(DisplayName = "Rest"),
    Flee        UMETA(DisplayName = "Flee"),
    Alert       UMETA(DisplayName = "Alert"),
    Interact    UMETA(DisplayName = "Interact"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Cautious    UMETA(DisplayName = "Cautious"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Curious     UMETA(DisplayName = "Curious"),
    Timid       UMETA(DisplayName = "Timid"),
    Territorial UMETA(DisplayName = "Territorial")
};

// ============================================================
// STRUCTS — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float StartHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState DesiredState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    FString RoutineLabel = TEXT("Default");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    FVector RoutineLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FNPC_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatStrength = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float TimeRecorded = 0.0f;
};

// ============================================================
// DELEGATE DECLARATIONS
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPC_BehaviorStateChangedDelegate,
    ENPC_BehaviorState, OldState,
    ENPC_BehaviorState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNPC_NPCDiedDelegate);

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- State Machine ----

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void TransitionToState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ForceState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetDead();

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    float GetThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RegisterThreatMemory(FVector ThreatLocation, float ThreatStrength);

    // ---- Delegates ----

    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_BehaviorStateChangedDelegate OnBehaviorStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_NPCDiedDelegate OnNPCDied;

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_PersonalityTrait PrimaryTrait;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float IdleDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolWaitDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_DailyRoutineEntry> DailyRoutine;

    // ---- Runtime State (Read-Only) ----

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    FVector PatrolDestination;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    FVector LastKnownThreatLocation;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    bool bHasThreatMemory;

private:
    bool bHomeLocationSet;
    float StateTimer;

    FTimerHandle BehaviorTickHandle;

    void EvaluateBehavior();
    void BuildDefaultDailyRoutine();

    void ExecutePatrolBehavior();
    void ExecuteFleeBehavior();
    void ExecuteAlertBehavior();
    void ExecuteForageBehavior();
    void ExecuteRestBehavior();

    APawn* DetectNearestThreat();
    FNPC_DailyRoutineEntry* GetCurrentRoutineEntry();
    FColor GetStateDebugColor() const;
};
