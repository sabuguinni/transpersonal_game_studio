// DinosaurBase.h
// Engine Architect #02 — PROD_CYCLE_AUTO_20260620_004
// Base class for all dinosaur pawns. Inherits ACharacter.
// All species (TRex, Raptor, Brachiosaurus, etc.) derive from this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// ── Enums at global scope (RULE 1) ──────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
};

UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead"),
};

// ── Class ────────────────────────────────────────────────────────────────────

UCLASS(Abstract, BlueprintType, Blueprintable)
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

    // ── Behavior state ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EDinosaurBehavior NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Behavior")
    EDinosaurBehavior GetBehaviorState() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Survival")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Survival")
    float GetHealthPercent() const;

protected:
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Survival")
    void OnDeath();
    virtual void OnDeath_Implementation() { OnDeath(); }

    // ── Species & stats ─────────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Identity")
    EDinosaurSpecies Species;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsAggressive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Survival")
    bool bIsAlive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EDinosaurBehavior BehaviorState;
};
