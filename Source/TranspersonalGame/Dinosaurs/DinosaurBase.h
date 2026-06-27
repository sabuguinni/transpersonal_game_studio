// DinosaurBase.h
// Transpersonal Game Studio — Engine Architect #02
// Base class for all dinosaur pawns in the prehistoric survival game.
// Provides: survival stats, AI perception, behavior state machine, locomotion.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "DinosaurBase.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinoClass : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
    Piscivore   UMETA(DisplayName = "Piscivore")
};

UENUM(BlueprintType)
enum class EDinoBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Alert       UMETA(DisplayName = "Alert"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead")
};

// ─── DinosaurBase ─────────────────────────────────────────────────────────────

UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = "Dinosaurs",
    meta = (DisplayName = "Dinosaur Base"))
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

    // ── Species Identity ──────────────────────────────────────────────────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Identity")
    FName SpeciesName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Identity")
    EDinoClass DinoClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Identity")
    bool bIsAggressive;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Identity")
    bool bIsPack;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Identity")
    int32 PackSize;

    // ── Health ────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float FleeThreshold;

    // ── Hunger & Thirst ───────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float CurrentHunger;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float MaxHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float CurrentThirst;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float MaxThirst;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float HungerDecayRate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float ThirstDecayRate;

    // ── Combat ────────────────────────────────────────────────────────────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dinosaur|Combat")
    float DetectionRange;

    // ── Behavior State ────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinoBehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsDead;

    // ── AI Perception ─────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* AIPerceptionComp;

    // ── Public API ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetBehaviorState(EDinoBehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    virtual void Die();

protected:
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|AI")
    void OnBehaviorStateChanged(EDinoBehaviorState OldState, EDinoBehaviorState NewState);
    virtual void OnBehaviorStateChanged_Implementation(EDinoBehaviorState OldState, EDinoBehaviorState NewState)
    {
        OnBehaviorStateChanged(OldState, NewState);
    }

    void UpdateLocomotionSpeed();
    void TickSurvivalStats();

private:
    FTimerHandle SurvivalDecayTimer;
};
