// DinosaurBase.h — Base class for all dinosaur types in Transpersonal Game
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260628_012
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DinosaurBase.generated.h"

// ─── Enums (global scope, before class) ───────────────────────────────────────

UENUM(BlueprintType)
enum class EEng_DinosaurSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
};

UENUM(BlueprintType)
enum class EEng_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Eating      UMETA(DisplayName = "Eating"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Investigating UMETA(DisplayName = "Investigating"),
};

UENUM(BlueprintType)
enum class EEng_DinosaurDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
};

// ─── Structs (global scope) ───────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FEng_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MoveSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float SprintSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Mass = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float HungerDecayRate = 0.5f;
};

USTRUCT(BlueprintType)
struct FEng_DinosaurSensoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Sensory")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Sensory")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Sensory")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Sensory")
    float SmellRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Sensory")
    bool bCanSmellBlood = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Sensory")
    bool bIsNocturnal = false;
};

// ─── Main Class ───────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = (TranspersonalGame), meta = (DisplayName = "Dinosaur Base"))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ── Core Stats ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinosaurSpecies Species = EEng_DinosaurSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinosaurDiet Diet = EEng_DinosaurDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Sensory")
    FEng_DinosaurSensoryData SensoryData;

    // ── Behavior State ──────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinosaurBehaviorState BehaviorState = EEng_DinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsAggressive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    float TerritoryRadius = 5000.0f;

    // ── Target Tracking ─────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    bool bIsInCombat = false;

    // ── Blueprint Events ────────────────────────────────────────────────────
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|Combat")
    void OnTakeDamage(float DamageAmount, AActor* DamageCauser);
    virtual void OnTakeDamage_Implementation(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|Combat")
    void OnDeath();
    virtual void OnDeath_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|Behavior")
    void OnTargetDetected(AActor* DetectedTarget);
    virtual void OnTargetDetected_Implementation(AActor* DetectedTarget);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|Behavior")
    void OnTargetLost();
    virtual void OnTargetLost_Implementation();

    // ── Callable Functions ──────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void ApplyDamage(float Amount, AActor* Causer);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EEng_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    bool CanHearTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    bool IsAlive() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // Internal hunger tick
    float HungerTickAccumulator = 0.0f;
};
