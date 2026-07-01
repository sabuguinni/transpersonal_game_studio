// DinosaurBase.h
// Transpersonal Game Studio — Core Systems Programmer #03
// Base class for all dinosaur pawns in the prehistoric survival world.
// Provides species traits, territory system, hunger/aggression AI hooks,
// and death/ragdoll delegate. All species inherit from this class.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "DinosaurBase.generated.h"

// ─── Enums (global scope — RULE 1) ────────────────────────────────────────────

/** Dinosaur species identifier. Drives InitializeSpecies() trait tables. */
UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

/** Dinosaur behavioral state machine. */
UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead")
};

// ─── Delegate ─────────────────────────────────────────────────────────────────

/** Broadcast when this dinosaur dies. Payload: the dead dinosaur actor. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDinosaurDeathDelegate, ADinosaurBase*, DeadDinosaur);

// ─── Class ────────────────────────────────────────────────────────────────────

/**
 * ADinosaurBase
 *
 * Abstract base pawn for all dinosaur species. Subclasses call
 * InitializeSpecies() in their constructor to load species-specific traits.
 *
 * Key systems:
 *  - Species trait table (HP, damage, speed, capsule)
 *  - Territory system (TerritoryCenter + TerritoryRadius)
 *  - Hunger drain → Foraging state transition
 *  - Aggression state on TakeDamage
 *  - Death → ragdoll physics + OnDinosaurDeath broadcast
 *  - AIPerceptionStimuliSource (Sight + Hearing)
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    // ── Constructor ──────────────────────────────────────────────────────────
    ADinosaurBase();

    // ── UE5 Lifecycle ────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Damage ───────────────────────────────────────────────────────────────
    /**
     * Override of AActor::TakeDamage. Reduces Health and switches to Attacking
     * state when hit. Triggers death when Health reaches 0.
     */
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    // ── Species Initialization ───────────────────────────────────────────────
    /**
     * Reads Species and populates MaxHealth, AttackDamage, MovementSpeed,
     * and capsule dimensions from the species trait table.
     * Must be called in the subclass constructor after setting Species.
     */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Species")
    void InitializeSpecies();

    // ── Territory ────────────────────────────────────────────────────────────
    /**
     * Returns true if WorldLocation is within TerritoryRadius of TerritoryCenter.
     */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Territory")
    bool IsInTerritory(const FVector& WorldLocation) const;

    // ── State Query ──────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|State")
    EDinosaurBehavior GetCurrentBehavior() const { return CurrentBehavior; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|State")
    bool IsAlive() const { return CurrentHealth > 0.0f; }

    // ── Delegate ─────────────────────────────────────────────────────────────
    /** Fired when this dinosaur dies. Subscribe in AI controllers or quest managers. */
    UPROPERTY(BlueprintAssignable, Category = "Dinosaur|Events")
    FOnDinosaurDeathDelegate OnDinosaurDeath;

    // ── Species Properties ───────────────────────────────────────────────────
    /** Which species this dinosaur is. Set in subclass constructor before calling InitializeSpecies(). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MovementSpeed;

    // ── Hunger ───────────────────────────────────────────────────────────────
    /** Current hunger level 0-100. Drains over time; triggers Foraging at HungerThreshold. */
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Survival")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float MaxHunger;

    /** Hunger drain rate per second. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float HungerDrainRate;

    /** Hunger level below which the dinosaur enters Foraging state. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float HungerThreshold;

    // ── Territory ────────────────────────────────────────────────────────────
    /** World-space center of this dinosaur's territory. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    FVector TerritoryCenter;

    /** Radius (cm) of this dinosaur's territory. Default 5000 cm = 50 m. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    // ── Behavior State ───────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State")
    EDinosaurBehavior CurrentBehavior;

    // ── Aggression ───────────────────────────────────────────────────────────
    /** True when the dinosaur is actively pursuing/attacking a target. */
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Combat")
    bool bIsAggressive;

    /** Aggression cooldown after losing sight of target (seconds). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AggressionCooldown;

protected:
    // ── Internal ─────────────────────────────────────────────────────────────
    /** Called when Health reaches 0. Enables ragdoll and broadcasts OnDinosaurDeath. */
    virtual void OnDinosaurDeathInternal();

    /** Aggression timer handle — resets bIsAggressive after cooldown. */
    FTimerHandle AggressionTimerHandle;

    /** AI Perception stimuli source — registers Sight and Hearing for AI perception. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
              meta = (AllowPrivateAccess = "true"))
    UAIPerceptionStimuliSourceComponent* PerceptionStimuliSource;
};
