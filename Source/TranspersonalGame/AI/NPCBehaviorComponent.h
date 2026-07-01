#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Graze       UMETA(DisplayName = "Graze"),
    Sleep       UMETA(DisplayName = "Sleep")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex         UMETA(DisplayName = "T-Rex"),
    Raptor       UMETA(DisplayName = "Raptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops  UMETA(DisplayName = "Triceratops"),
    Pterodactyl  UMETA(DisplayName = "Pterodactyl")
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_BehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float MemoryDuration = 15.0f;
};

USTRUCT(BlueprintType)
struct FNPC_StimulusMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float MemoryAge = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bIsValid = false;
};

// ============================================================
// DELEGATE
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FNPC_OnBehaviorStateChanged,
    ENPC_BehaviorState, OldState,
    ENPC_BehaviorState, NewState
);

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPC_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorComponent();

    // ---- State ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FNPC_BehaviorConfig BehaviorConfig;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FNPC_StimulusMemory StimulusMemory;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    bool bIsAlerted;

    // ---- Events ----
    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_OnBehaviorStateChanged OnBehaviorStateChanged;

    // ---- Blueprint callable ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetSpecies(ENPC_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ForceState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    FString GetCurrentStateName() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FVector PatrolHomeLocation;
    FVector CurrentPatrolTarget;
    float AttackCooldown;
    float StateTimeElapsed;

    void ApplySpeciesConfig();
    void UpdateStateMachine(float DeltaTime);
    APawn* SensePlayer();

    void UpdateIdle(float DeltaTime, APawn* Player);
    void UpdatePatrol(float DeltaTime, APawn* Player);
    void UpdateInvestigate(float DeltaTime, APawn* Player);
    void UpdateChase(float DeltaTime, APawn* Player);
    void UpdateAttack(float DeltaTime, APawn* Player);
    void UpdateFlee(float DeltaTime, APawn* Player);
    void UpdateGraze(float DeltaTime, APawn* Player);
    void UpdateSleep(float DeltaTime, APawn* Player);

    void TransitionToState(ENPC_BehaviorState NewState);
    void PickNewPatrolTarget();
    void ExecuteAttack(APawn* Target);
};
