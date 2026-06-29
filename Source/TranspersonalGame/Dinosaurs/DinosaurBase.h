// DinosaurBase.h
// Transpersonal Game Studio — Engine Architect #02
// Base class for all dinosaur species. All dinosaur types inherit from this.
// Implements: health, stamina, hunger, combat, death, AI perception stimuli.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DinosaurBase.generated.h"

// ============================================================
// Enums — declared at global scope (UE5 rule)
// ============================================================

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Unknown           UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex  UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor      UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus     UMETA(DisplayName = "Brachiosaurus"),
    Triceratops       UMETA(DisplayName = "Triceratops"),
    Stegosaurus       UMETA(DisplayName = "Stegosaurus"),
    Pterodactyl       UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus      UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus       UMETA(DisplayName = "Spinosaurus")
};

UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead")
};

// ============================================================
// Forward declarations
// ============================================================

class UAIPerceptionStimuliSourceComponent;

// ============================================================
// ADinosaurBase
// ============================================================

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

    // --------------------------------------------------------
    // Species Identity
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurDiet DietType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FString SpeciesDisplayName;

    // --------------------------------------------------------
    // Health
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Health")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Health")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Health")
    bool bIsDead;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Health")
    void ApplyDamage_Dino(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    bool IsAlive() const;

    // --------------------------------------------------------
    // Stamina
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stamina")
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stamina")
    float CurrentStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stamina")
    float StaminaRegenRate;

    // --------------------------------------------------------
    // Hunger
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Hunger")
    float MaxHunger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Hunger")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Hunger")
    float HungerDrainRate;

    // --------------------------------------------------------
    // Movement
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float TurnRate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Movement")
    bool bIsRunning;

    // --------------------------------------------------------
    // Combat
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    bool bIsAggressive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    bool bIsAlerted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AggressionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float DetectionRadius;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void OnAlerted(AActor* Threat);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Combat")
    bool IsWithinDetectionRange(AActor* OtherActor) const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Combat")
    bool IsWithinAggressionRange(AActor* OtherActor) const;

    // --------------------------------------------------------
    // Utility
    // --------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Utility")
    FString GetSpeciesName() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Utility")
    virtual void Die();

protected:
    virtual void OnDeath();

    // --------------------------------------------------------
    // AI Perception
    // --------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionStimuliSourceComponent* PerceptionStimuliSource;

    // --------------------------------------------------------
    // Internal timers
    // --------------------------------------------------------

    FTimerHandle HungerTimerHandle;
    FTimerHandle StaminaTimerHandle;
    float LastAttackTime;

    void DrainHunger();
    void RegenStamina();
};
