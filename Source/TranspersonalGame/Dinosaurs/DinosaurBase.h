// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur species. Provides state machine, survival stats,
// patrol, detection, and combat. Species-specific data set in child Blueprints.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Dino AI state — global scope, unique prefix to avoid ODR conflicts
// ─────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────
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

    // ── Species identity ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FString SpeciesName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsHerbivore = false;

    // ── Health & Stamina ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina = 200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentStamina = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float StaminaDrainRate = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float StaminaRegenRate = 10.0f;

    // ── Combat ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown = 2.0f;

    // ── Movement ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float ChaseSpeed = 600.0f;

    // ── Detection ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float DetectionRadius = 1500.0f;

    // ── State machine ────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinoState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    TObjectPtr<AActor> TargetActor;

    // ── Patrol waypoints ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    TArray<FVector> PatrolWaypoints;

    // ── Public API ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDamageFromSource(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void TransitionToState(EDinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const { return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    bool IsDead() const { return CurrentState == EDinoState::Dead; }

    // ── Blueprint events ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnStateChanged(EDinoState NewState);
    virtual void OnStateChanged_Implementation(EDinoState NewState);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnAttackPerformed(float DamageDealt);
    virtual void OnAttackPerformed_Implementation(float DamageDealt);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath();
    virtual void OnDeath_Implementation();

private:
    // State tick helpers
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickFlee(float DeltaTime);

    void PerformAttack();
    void UpdateStamina(float DeltaTime);
    AActor* DetectThreat() const;

    void GeneratePatrolWaypoints(int32 Count, float Radius);
    void AdvancePatrolTarget();

    // Internal state
    FVector HomeLocation;
    int32   PatrolIndex            = 0;
    float   StateTimer             = 0.0f;
    float   IdleDuration           = 5.0f;
    float   AttackCooldownRemaining = 0.0f;
};
