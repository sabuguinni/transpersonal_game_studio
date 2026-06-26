#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "DinosaurBehaviorComponent.generated.h"

// ============================================================
// Enums — must be at global scope (UE5 compilation rule #1)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Generic         UMETA(DisplayName = "Generic")
};

// ============================================================
// Structs — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_DinoSensoryData
{
    GENERATED_BODY()

    /** Distance at which the dinosaur detects the player by sight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    float SightRange = 3000.0f;

    /** Distance at which the dinosaur detects the player by sound */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    float HearingRange = 1500.0f;

    /** Distance at which the dinosaur detects the player by smell */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    float SmellRange = 800.0f;

    /** Field of view in degrees for sight detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensory")
    float SightFOV = 120.0f;

    /** Whether the player is currently detected */
    UPROPERTY(BlueprintReadOnly, Category = "Sensory")
    bool bPlayerDetected = false;

    /** Last known player location */
    UPROPERTY(BlueprintReadOnly, Category = "Sensory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    /** Time since player was last seen (seconds) */
    UPROPERTY(BlueprintReadOnly, Category = "Sensory")
    float TimeSinceLastSeen = 0.0f;
};

USTRUCT(BlueprintType)
struct FNPC_DinoPatrolData
{
    GENERATED_BODY()

    /** Radius around home location for patrol */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolRadius = 5000.0f;

    /** Home/spawn location — patrol stays within radius of this */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector HomeLocation = FVector::ZeroVector;

    /** Current patrol target */
    UPROPERTY(BlueprintReadOnly, Category = "Patrol")
    FVector CurrentPatrolTarget = FVector::ZeroVector;

    /** How long to wait at each patrol point (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float WaitTimeAtPoint = 3.0f;

    /** Current wait timer */
    UPROPERTY(BlueprintReadOnly, Category = "Patrol")
    float CurrentWaitTimer = 0.0f;

    /** Whether currently waiting at a patrol point */
    UPROPERTY(BlueprintReadOnly, Category = "Patrol")
    bool bIsWaiting = false;
};

USTRUCT(BlueprintType)
struct FNPC_DinoCombatData
{
    GENERATED_BODY()

    /** Melee attack range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    /** Damage dealt per attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 40.0f;

    /** Cooldown between attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    /** Current attack cooldown timer */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float AttackCooldownTimer = 0.0f;

    /** Maximum chase distance before giving up */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxChaseDistance = 8000.0f;

    /** Health below which the dinosaur flees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.2f;

    /** Current health (0.0 - 1.0 normalized) */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float CurrentHealthNormalized = 1.0f;
};

// ============================================================
// Main Component
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Behavior Component")
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

    // ---- Species & State ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::Generic;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State")
    ENPC_DinoState CurrentState = ENPC_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State")
    ENPC_DinoState PreviousState = ENPC_DinoState::Idle;

    // ---- Sensory Data ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Sensory")
    FNPC_DinoSensoryData SensoryData;

    // ---- Patrol Data ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    FNPC_DinoPatrolData PatrolData;

    // ---- Combat Data ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    FNPC_DinoCombatData CombatData;

    // ---- Behavior Flags ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsAggressive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsHerbivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bEnablePatrol = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bEnableDebugDraw = false;

    // ---- Public API ----

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetState(ENPC_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    ENPC_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Sensory")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Sensory")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDamageNormalized(float DamageNormalized);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void ApplySpeciesDefaults();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Patrol")
    FVector PickNewPatrolTarget();

    UFUNCTION(BlueprintPure, Category = "Dinosaur|State")
    bool IsInCombatState() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|State")
    bool IsFleeingState() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void UpdateSensory(float DeltaTime);
    void UpdateStateMachine(float DeltaTime);
    void UpdatePatrol(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateFlee(float DeltaTime);

    UPROPERTY()
    APawn* CachedPlayerPawn = nullptr;

    float StateTimer = 0.0f;
};
