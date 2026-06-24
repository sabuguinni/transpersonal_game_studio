// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur pawns in the prehistoric survival game.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// ─── Dino AI state machine ────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EDinoState : uint8
{
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Returning   UMETA(DisplayName = "Returning"),
    Dead        UMETA(DisplayName = "Dead")
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
    virtual void Tick(float DeltaTime) override;

    // ── Survival stats ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackCooldown;

    // ── Territory & detection ─────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float ThreatDetectRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    bool bIsAggressive;

    // ── Movement speeds ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float ChaseSpeed;

    // ── State machine ─────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinoState CurrentState;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetDinoState(EDinoState NewState);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|AI")
    void OnDinoStateChanged(EDinoState NewState);
    virtual void OnDinoStateChanged_Implementation(EDinoState NewState) { OnDinoStateChanged(NewState); }

    // ── Damage override ───────────────────────────────────────────────────
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

private:
    // Home position (set at BeginPlay)
    FVector HomeLocation;

    // Current patrol waypoint
    FVector CurrentWaypoint;

    // Timer for cheap AI ticks
    FTimerHandle PatrolTimerHandle;

    // Last attack timestamp
    float LastAttackTime = 0.f;

    // Internal helpers
    void UpdateBehavior();
    void DoPatrol();
    void TryAttackPlayer(APawn* PlayerPawn);
};
