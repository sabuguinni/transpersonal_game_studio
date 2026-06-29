#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DinosaurBase.generated.h"

/** Dinosaur species classification */
UENUM(BlueprintType)
enum class EEng_DinoSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
};

/** Dinosaur behavior state */
UENUM(BlueprintType)
enum class EEng_DinoBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Eating      UMETA(DisplayName = "Eating"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
};

/** Diet classification */
UENUM(BlueprintType)
enum class EEng_DinoDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
};

/** Core stats for a dinosaur */
USTRUCT(BlueprintType)
struct FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RunSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Mass = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float HungerDecayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHunger = 100.0f;
};

/**
 * ADinosaurBase — Base class for all dinosaur species in the prehistoric survival game.
 * Inherits from ACharacter to leverage UE5 movement, capsule collision, and animation.
 * All species (T-Rex, Raptor, Brachiosaurus, etc.) derive from this class.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ─── Species & Classification ───────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinoSpecies Species = EEng_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinoDiet Diet = EEng_DinoDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName DinosaurName = NAME_None;

    // ─── Stats ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinoStats Stats;

    // ─── Behavior State ──────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinoBehaviorState BehaviorState = EEng_DinoBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    float TerritoryRadius = 3000.0f;

    // ─── Combat ──────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    bool bIsAlive = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    AActor* CurrentTarget = nullptr;

    // ─── Blueprint-callable Functions ────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual float TakeDamageFromPlayer(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    virtual void SetBehaviorState(EEng_DinoBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    virtual void OnPlayerDetected(AActor* Player);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    bool IsAlive() const { return bIsAlive && Stats.CurrentHealth > 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void HealDinosaur(float HealAmount);

protected:
    // Internal helpers
    void UpdateHunger(float DeltaTime);
    void CheckForNearbyPlayers();
    void Die();

    // Tick timer accumulator
    float HungerTickAccumulator = 0.0f;
    float DetectionTickAccumulator = 0.0f;
};
