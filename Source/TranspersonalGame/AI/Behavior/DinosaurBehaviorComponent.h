#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurBehaviorComponent.generated.h"

// ============================================================
// Enums — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_DinoAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Eating      UMETA(DisplayName = "Eating"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Dilophosaurus       UMETA(DisplayName = "Dilophosaurus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl")
};

UENUM(BlueprintType)
enum class ENPC_DinoDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore")
};

// ============================================================
// Structs — must be at global scope (UHT rule)
// ============================================================

/** Defines the territory a dinosaur patrols */
USTRUCT(BlueprintType)
struct FNPC_DinoTerritory
{
    GENERATED_BODY()

    /** Centre of the territory in world space */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector Centre = FVector::ZeroVector;

    /** Radius within which the dinosaur patrols */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float PatrolRadius = 2000.0f;

    /** Radius within which the dinosaur becomes aggressive */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float AggressionRadius = 3000.0f;

    /** Radius within which the dinosaur attacks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float AttackRadius = 300.0f;

    /** Ordered list of patrol waypoints within the territory */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolWaypoints;
};

/** Memory entry — the dinosaur remembers recent stimuli */
USTRUCT(BlueprintType)
struct FNPC_DinoMemoryEntry
{
    GENERATED_BODY()

    /** World location of the stimulus */
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FVector Location = FVector::ZeroVector;

    /** Game time when the stimulus was detected */
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float DetectedAtTime = 0.0f;

    /** How threatening this stimulus was (0=neutral, 1=max threat) */
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float ThreatLevel = 0.0f;

    /** Whether this memory is still considered fresh */
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bIsFresh = false;
};

/** Per-species trait data */
USTRUCT(BlueprintType)
struct FNPC_DinoSpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    ENPC_DinoDiet Diet = ENPC_DinoDiet::Carnivore;

    /** Base movement speed (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float RunSpeed = 700.0f;

    /** Sight range in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float SightRange = 3000.0f;

    /** Hearing range in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float HearingRange = 2000.0f;

    /** Horizontal field of view in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float FieldOfViewDegrees = 120.0f;

    /** Whether this species hunts in packs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    bool bIsPack = false;

    /** Max pack size */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    int32 MaxPackSize = 1;

    /** Base damage per attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float AttackDamage = 50.0f;

    /** Time between attacks in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float AttackCooldown = 2.0f;

    /** How long the dinosaur can sustain a chase before disengaging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traits")
    float MaxChaseDuration = 30.0f;
};

// ============================================================
// Component
// ============================================================

/**
 * UDinosaurBehaviorComponent
 *
 * Drives the NPC AI state machine for all dinosaur species.
 * Attach to any APawn that represents a dinosaur.
 *
 * Responsibilities:
 *   - Maintains current AI state (ENPC_DinoAIState)
 *   - Manages territory definition and patrol waypoints
 *   - Runs perception (sight + hearing) checks each tick
 *   - Maintains a short-term memory of recent stimuli
 *   - Writes AggressionLevel (0-1) consumed by DinosaurAnimInstance
 *   - Broadcasts state-change events for BT/Blueprint integration
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

    // ---- UActorComponent overrides ----
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Public API ----

    /** Called by damage system when the dinosaur takes a hit */
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void OnDamageTaken(float DamageAmount, AActor* DamageInstigator);

    /** Called by the death system to trigger death state */
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void OnDeath();

    /** Force the dinosaur to investigate a world location */
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InvestigateLocation(const FVector& Location, float ThreatLevel = 0.5f);

    /** Returns current AI state */
    UFUNCTION(BlueprintPure, Category = "Behavior")
    ENPC_DinoAIState GetCurrentState() const { return CurrentState; }

    /** Returns aggression level 0-1 (consumed by DinosaurAnimInstance) */
    UFUNCTION(BlueprintPure, Category = "Behavior")
    float GetAggressionLevel() const { return AggressionLevel; }

    /** Returns current patrol waypoint index */
    UFUNCTION(BlueprintPure, Category = "Behavior")
    int32 GetCurrentWaypointIndex() const { return CurrentWaypointIndex; }

    /** Returns the next patrol waypoint in world space */
    UFUNCTION(BlueprintPure, Category = "Behavior")
    FVector GetNextPatrolWaypoint() const;

    /** Advance to the next waypoint in the patrol sequence */
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AdvanceToNextWaypoint();

    /** Returns true if the target actor is within sight */
    UFUNCTION(BlueprintPure, Category = "Perception")
    bool CanSeeActor(const AActor* TargetActor) const;

    /** Returns true if the target actor is within hearing range */
    UFUNCTION(BlueprintPure, Category = "Perception")
    bool CanHearActor(const AActor* TargetActor) const;

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FNPC_DinoSpeciesTraits SpeciesTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FNPC_DinoTerritory Territory;

    /** How long a memory entry remains fresh (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDuration = 15.0f;

    /** Maximum number of memory entries retained */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries = 8;

    /** Current health ratio 0-1 (written by damage system, read by AnimInstance) */
    UPROPERTY(BlueprintReadWrite, Category = "Health")
    float HealthRatio = 1.0f;

    // ---- Delegates ----

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPC_OnStateChanged,
        ENPC_DinoAIState, OldState, ENPC_DinoAIState, NewState);

    UPROPERTY(BlueprintAssignable, Category = "Behavior|Events")
    FNPC_OnStateChanged OnStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_OnTargetAcquired, AActor*, Target);

    UPROPERTY(BlueprintAssignable, Category = "Behavior|Events")
    FNPC_OnTargetAcquired OnTargetAcquired;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNPC_OnDinoAttack);

    UPROPERTY(BlueprintAssignable, Category = "Behavior|Events")
    FNPC_OnDinoAttack OnAttackExecuted;

private:
    // ---- Internal state ----

    UPROPERTY()
    ENPC_DinoAIState CurrentState = ENPC_DinoAIState::Idle;

    UPROPERTY()
    float AggressionLevel = 0.0f;

    UPROPERTY()
    TWeakObjectPtr<AActor> CurrentTarget;

    UPROPERTY()
    TArray<FNPC_DinoMemoryEntry> MemoryEntries;

    int32 CurrentWaypointIndex = 0;

    float ChaseElapsedTime = 0.0f;
    float AttackCooldownRemaining = 0.0f;
    float IdleTimer = 0.0f;

    // ---- Internal methods ----

    void TransitionToState(ENPC_DinoAIState NewState);
    void UpdatePerception();
    void UpdateStateMachine(float DeltaTime);
    void UpdateAggressionLevel(float DeltaTime);
    void PruneStaleMemories();
    void AddMemoryEntry(const FVector& Location, float ThreatLevel);
    FVector GetMostThreateningMemoryLocation() const;

    bool IsPlayerWithinRange(float Range) const;
    AActor* FindPlayerActor() const;
};
