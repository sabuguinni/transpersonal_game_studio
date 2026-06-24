// TRexCharacter.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260624_001
// Tyrannosaurus Rex — apex predator dinosaur character
// Derives from ACharacter (UE5 standard) — no custom movement system

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TRexCharacter.generated.h"

UENUM(BlueprintType)
enum class EWorld_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Roaring     UMETA(DisplayName = "Roaring"),
};

USTRUCT(BlueprintType)
struct FWorld_TRexStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float Health = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float MaxHealth = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float AttackDamage = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float DetectionRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float AttackRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float WalkSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float RunSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float StompDamageRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float StompDamage = 150.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRexCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Combat ──────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformBite();

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStomp();

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void Roar();

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    float ApplyDamage(float DamageAmount);

    // ── State ────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    void SetBehaviourState(EWorld_TRexState NewState);

    UFUNCTION(BlueprintPure, Category = "TRex|State")
    EWorld_TRexState GetBehaviourState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "TRex|State")
    bool IsAlive() const { return Stats.Health > 0.0f; }

    UFUNCTION(BlueprintPure, Category = "TRex|State")
    bool IsHunting() const { return CurrentState == EWorld_TRexState::Hunting; }

    // ── Detection ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "TRex|AI")
    AActor* FindNearestPrey() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|AI")
    bool CanSeeActor(AActor* Target) const;

    // ── Stats ────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    FWorld_TRexStats Stats;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State",
              meta = (AllowPrivateAccess = "true"))
    EWorld_TRexState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    float PatrolRadius = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    float HungerDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    bool bIsAlpha = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    float RoarCooldown = 30.0f;

private:
    float LastRoarTime = 0.0f;
    float StateTimer = 0.0f;

    void UpdateHunger(float DeltaTime);
    void UpdateBehaviour(float DeltaTime);
    void TransitionToHunting();
};
