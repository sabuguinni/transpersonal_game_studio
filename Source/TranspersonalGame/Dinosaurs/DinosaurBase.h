// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur species in the game.
// Inherits from ACharacter to leverage UE5 movement + capsule collision.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "DinosaurBase.generated.h"

// Dinosaur AI state machine — must be at global scope (RULE 1)
UENUM(BlueprintType)
enum class EDinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Dead        UMETA(DisplayName = "Dead")
};

// Dinosaur diet type
UENUM(BlueprintType)
enum class EDinoDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore")
};

/**
 * ADinosaurBase
 * Base class for all dinosaur species.
 * Provides: health, hunger, AI state machine, detection, combat, death.
 * All species (TRex, Raptor, Brachiosaurus, etc.) inherit from this.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // =====================================================================
    // SPECIES IDENTITY
    // =====================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsCarnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    int32 PackSize;

    // =====================================================================
    // HEALTH & COMBAT STATS
    // =====================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange;

    // =====================================================================
    // MOVEMENT SPEEDS
    // =====================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float RunSpeed;

    // =====================================================================
    // DETECTION & TERRITORY
    // =====================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float AggroRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float TerritoryRadius;

    // =====================================================================
    // HUNGER SYSTEM
    // =====================================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float MaxHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float HungerDecayRate;

    // =====================================================================
    // STATE
    // =====================================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinoState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsDead;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsAggressive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* CurrentTarget;

    // =====================================================================
    // DEBUG
    // =====================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Debug")
    bool bDrawDebugSpheres;

    // =====================================================================
    // COMPONENTS
    // =====================================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionStimuliSourceComponent* PerceptionStimuliSource;

    // =====================================================================
    // PUBLIC INTERFACE
    // =====================================================================

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetCurrentTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void DetectNearbyThreats();

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|AI")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|AI")
    EDinoState GetCurrentState() const;

    // =====================================================================
    // BLUEPRINT EVENTS (overridable in BP)
    // =====================================================================

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|Events")
    void OnDinosaurDamaged(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|Events")
    void OnDinosaurDied();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|Events")
    void OnDinosaurAttack(AActor* Target);

protected:
    void Die();
    void UpdateHunger();
    void UpdateBehaviour();

    FTimerHandle HungerTimerHandle;
    FTimerHandle BehaviourTimerHandle;
};
