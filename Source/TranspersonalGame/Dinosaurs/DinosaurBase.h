#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Dead        UMETA(DisplayName = "Dead"),
};

UENUM(BlueprintType)
enum class EEng_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
};

USTRUCT(BlueprintType)
struct FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackCooldown = 2.0f;
};

/**
 * ADinosaurBase — Base class for all dinosaur AI actors.
 * Inherits ACharacter for movement + capsule collision.
 * State machine: Idle → Patrol → Chase → Attack → Flee → Dead
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Species & Stats ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Identity")
    EEng_DinoSpecies Species = EEng_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    FEng_DinoStats Stats;

    // ── State Machine ─────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|AI")
    EEng_DinoState CurrentState = EEng_DinoState::Idle;

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    void SetDinoState(EEng_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    EEng_DinoState GetDinoState() const { return CurrentState; }

    // ── Combat ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dino|Combat")
    void TakeDamageFromPlayer(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Dino|Combat")
    bool IsAlive() const { return Stats.CurrentHealth > 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Dino|Combat")
    void PerformAttack();

    // ── Patrol ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Patrol")
    float PatrolRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Patrol")
    float PatrolWaitTime = 3.0f;

    // ── Events (override in subclasses) ──────────────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "Dino|Events")
    void OnDinoStateChanged(EEng_DinoState OldState, EEng_DinoState NewState);
    virtual void OnDinoStateChanged_Implementation(EEng_DinoState OldState, EEng_DinoState NewState);

    UFUNCTION(BlueprintNativeEvent, Category = "Dino|Events")
    void OnDinoDied();
    virtual void OnDinoDied_Implementation();

protected:
    // ── Internal AI tick ─────────────────────────────────────────────────
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickFlee(float DeltaTime);

    AActor* FindNearestPlayer() const;
    float   GetDistanceToPlayer() const;
    void    MoveTowardsPlayer(float DeltaTime);
    void    PickNewPatrolTarget();

    UPROPERTY()
    AActor* TargetPlayer = nullptr;

    UPROPERTY()
    FVector PatrolTarget = FVector::ZeroVector;

    FVector SpawnLocation = FVector::ZeroVector;

    float AttackCooldownRemaining = 0.0f;
    float PatrolWaitRemaining     = 0.0f;
    float IdleTimer               = 0.0f;
};
