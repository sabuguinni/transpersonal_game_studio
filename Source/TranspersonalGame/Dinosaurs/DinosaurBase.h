// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur pawns in the prehistoric survival game.
// All species inherit from this and override ApplySpeciesStats().

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DinosaurBase.generated.h"

// ─── Enumerations (global scope — UHT requirement) ────────────────────────────

UENUM(BlueprintType)
enum class EDinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alert       UMETA(DisplayName = "Alert"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EDinoSpecies : uint8
{
    Generic         UMETA(DisplayName = "Generic"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus")
};

// ─── DinosaurBase ─────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ─── Combat ──────────────────────────────────────────────────────────────

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void Die();

    // ─── Behaviour ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EDinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void UpdateBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void ApplySpeciesStats();

    // ─── Stats (editable per instance) ───────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    EDinoSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float RunSpeed;

    // ─── State ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State")
    EDinoState CurrentBehaviorState;

    UPROPERTY(BlueprintReadWrite, Category = "Dinosaur|State")
    bool bIsAggressive;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsAlerted;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsDead;

private:
    FTimerHandle BehaviorTickHandle;
};
