// DinosaurBase.h
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260701_010
// Base class for all dinosaur pawns. Inherits ACharacter for movement + animation.
// All dinosaur species (TRex, Raptor, Brachiosaurus, etc.) inherit from this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// Forward declarations
class UAIPerceptionStimuliSourceComponent;

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — must be at global scope (UHT rule)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Unknown            UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex   UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor       UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus      UMETA(DisplayName = "Brachiosaurus"),
    Triceratops        UMETA(DisplayName = "Triceratops"),
    Pterodactyl        UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus       UMETA(DisplayName = "Ankylosaurus"),
    Stegosaurus        UMETA(DisplayName = "Stegosaurus"),
    Spinosaurus        UMETA(DisplayName = "Spinosaurus"),
    Parasaurolophus    UMETA(DisplayName = "Parasaurolophus"),
};

UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle       UMETA(DisplayName = "Idle"),
    Wandering  UMETA(DisplayName = "Wandering"),
    Hunting    UMETA(DisplayName = "Hunting"),
    Fleeing    UMETA(DisplayName = "Fleeing"),
    Attacking  UMETA(DisplayName = "Attacking"),
    Feeding    UMETA(DisplayName = "Feeding"),
    Resting    UMETA(DisplayName = "Resting"),
    Dead       UMETA(DisplayName = "Dead"),
};

// ─────────────────────────────────────────────────────────────────────────────
// ADinosaurBase
// ─────────────────────────────────────────────────────────────────────────────

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

    // ── Species & Identity ────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName DinosaurName;

    // ── Health ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    // ── Combat ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    bool bIsAggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    bool bIsPredator;

    // ── Behavior ──────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EDinosaurBehavior BehaviorState;

    // ── Survival Stats ────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float HungerLevel;   // 0=full, 1=starving

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float ThirstLevel;   // 0=hydrated, 1=dehydrated

    // ── AI Perception ─────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    UAIPerceptionStimuliSourceComponent* PerceptionStimuliSource;

    // ── Blueprint-callable functions ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    EDinosaurBehavior GetBehaviorState() const { return BehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EDinosaurBehavior NewState) { BehaviorState = NewState; }

protected:
    // Apply species-specific stats (capsule size, speed, health, etc.)
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Identity")
    virtual void ApplySpeciesDefaults();

    // Called every 2 seconds to update AI behavior state
    UFUNCTION()
    void UpdateBehaviorState();

    // Called when health reaches 0
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Combat")
    void OnDeath();
    virtual void OnDeath_Implementation() { OnDeath(); }

private:
    FTimerHandle BehaviorTickHandle;
};
