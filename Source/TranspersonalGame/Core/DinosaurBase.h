// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur pawns. Inherits ACharacter for movement + animation.
// All species subclasses MUST inherit from ADinosaurBase.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "DinosaurBase.generated.h"

// ---------------------------------------------------------------------------
// Enums — GLOBAL SCOPE (RULE 1: USTRUCT/UENUM at global scope only)
// Prefixed with EEng_ to avoid collisions with other agents (RULE 2)
// ---------------------------------------------------------------------------

UENUM(BlueprintType)
enum class EEng_DinosaurSpecies : uint8
{
    Unknown             UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus  UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops       UMETA(DisplayName = "Protoceratops"),
};

UENUM(BlueprintType)
enum class EEng_DinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Dead        UMETA(DisplayName = "Dead"),
};

// ---------------------------------------------------------------------------
// ADinosaurBase
// ---------------------------------------------------------------------------

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

    // --- Health ---
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                              AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Health")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Species")
    FString GetSpeciesName() const;

    // --- Species ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EEng_DinosaurSpecies DinosaurSpecies;

    // --- Behavior ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinosaurBehavior BehaviorState;

    // --- Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius;

    // --- Ecology ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Ecology")
    bool bIsHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Ecology")
    bool bIsPack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    float PatrolRadius;

protected:
    // Home location for patrol wander
    FVector HomeLocation;

    // Behavior tick timer
    FTimerHandle BehaviorTickHandle;

    UFUNCTION()
    void UpdateBehavior();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath();
    virtual void OnDeath_Implementation() { OnDeath(); }

private:
    // Called internally when health reaches 0
    void OnDeath();
};
