#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
};

UENUM(BlueprintType)
enum class EEng_DinoBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Dead        UMETA(DisplayName = "Dead"),
};

USTRUCT(BlueprintType)
struct FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float RunSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float HungerDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    bool bIsPackHunter = false;
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Stats ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FEng_DinoStats DinoStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EEng_DinoSpecies Species = EEng_DinoSpecies::TyrannosaurusRex;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur")
    EEng_DinoBehaviorState BehaviorState = EEng_DinoBehaviorState::Idle;

    // ── Combat ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDamageFromPlayer(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    float GetHealthPercent() const;

    // ── Behavior ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EEng_DinoBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void DetectNearbyPlayer();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void UpdateHunger(float DeltaTime);

    // ── Events (override in Blueprint) ───────────────────────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnDinosaurDeath();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnDinosaurAttack(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnBehaviorStateChanged(EEng_DinoBehaviorState OldState, EEng_DinoBehaviorState NewState);

protected:
    // ── Internal ─────────────────────────────────────────────────────────
    UPROPERTY()
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float BehaviorTickInterval = 0.5f;

    float BehaviorTickAccumulator = 0.0f;

    void TickBehavior(float DeltaTime);
    void HandleIdleBehavior();
    void HandleHuntingBehavior();
    void HandleFleeingBehavior();
    void Die();
};
