// DinosaurBase.h
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260701_004
// Base class for all dinosaur types. Provides AI state machine, perception,
// health/stamina, attack, territory, and species-specific override hooks.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "DinosaurBase.generated.h"

// AI state machine for dinosaur behavior
UENUM(BlueprintType)
enum class EDinoAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead")
};

// Dinosaur species category
UENUM(BlueprintType)
enum class EDinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Generic             UMETA(DisplayName = "Generic")
};

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Dinosaurs))
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

    // ─── AI PERCEPTION ───────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* AIPerceptionComp;

    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ─── HEALTH & STAMINA ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentStamina;

    // ─── COMBAT ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown;

    // ─── MOVEMENT ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float RunSpeed;

    // ─── TERRITORY & DETECTION ───────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float ChaseRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    FVector HomeLocation;

    // ─── SPECIES TRAITS ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    bool bIsHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    bool bIsPack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    int32 PackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EDinoSpecies Species;

    // ─── AI STATE ────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinoAIState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsAlive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsAttacking;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* TargetActor;

    // ─── PUBLIC FUNCTIONS ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetAIState(EDinoAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void Die();

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetStaminaPercent() const { return MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur|AI")
    bool IsAlive() const { return bIsAlive; }

    // ─── BLUEPRINT EVENTS ────────────────────────────────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnAttack(AActor* Target);
    virtual void OnAttack_Implementation(AActor* Target);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDamageReceived(float Damage, AActor* DamageCauser);
    virtual void OnDamageReceived_Implementation(float Damage, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath();
    virtual void OnDeath_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnStateChanged(EDinoAIState OldState, EDinoAIState NewState);
    virtual void OnStateChanged_Implementation(EDinoAIState OldState, EDinoAIState NewState);

private:
    // ─── INTERNAL AI HANDLERS ────────────────────────────────────────────────
    void UpdateAIState();
    void HandleIdleState();
    void HandlePatrolState();
    void HandleChaseState();
    void HandleAttackState();
    void HandleFleeState();
    void HandleFeedState();
    void HandleRestState();
    void RegenerateStamina();

    void MoveToLocation(const FVector& Location);
    void MoveToActor(AActor* Actor);

    FTimerHandle AIUpdateTimer;
    FTimerHandle StaminaRegenTimer;
    float LastAttackTime;
};
