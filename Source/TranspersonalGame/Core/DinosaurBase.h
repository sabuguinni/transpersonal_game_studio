// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur AI pawns in the prehistoric survival game
// Cycle: PROD_CYCLE_AUTO_20260624_006

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EDinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
};

UENUM(BlueprintType)
enum class EDinoState : uint8
{
    Idle    UMETA(DisplayName = "Idle"),
    Patrol  UMETA(DisplayName = "Patrol"),
    Chase   UMETA(DisplayName = "Chase"),
    Attack  UMETA(DisplayName = "Attack"),
    Flee    UMETA(DisplayName = "Flee"),
    Dead    UMETA(DisplayName = "Dead"),
};

// ============================================================
// UCLASS
// ============================================================

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
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // ---- Species & State ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinoSpecies Species;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    EDinoState CurrentState;

    // ---- Combat Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRadius;

    // ---- Behaviour ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    float PatrolWaitTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    bool bIsAggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    bool bIsPackHunter;

    // ---- Target ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* TargetActor;

    // ---- Blueprint Events ----
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnStateChanged(EDinoState OldState, EDinoState NewState);
    virtual void OnStateChanged_Implementation(EDinoState OldState, EDinoState NewState);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnAttackPerformed();
    virtual void OnAttackPerformed_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinosaurDied();
    virtual void OnDinosaurDied_Implementation();

    // ---- Utility ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Utility")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Utility")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Utility")
    FString GetSpeciesName() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetState(EDinoState NewState);

private:
    // Internal state machine ticks
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickFlee(float DeltaTime);

    void ScanForTargets();
    void PerformAttack();
    void PickNewPatrolPoint();
    void Die();

    FVector HomeLocation;
    FVector CurrentPatrolTarget;
    float TimeSinceLastAttack;
    float PatrolWaitTimer;
};
