#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

// ============================================================
// Dinosaur species enum — unique prefix to avoid ODR conflicts
// ============================================================
UENUM(BlueprintType)
enum class EDB_DinosaurSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pteranodon      UMETA(DisplayName = "Pteranodon"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
};

// ============================================================
// Dinosaur behaviour state
// ============================================================
UENUM(BlueprintType)
enum class EDB_BehaviourState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Attacking   UMETA(DisplayName = "Attacking"),
};

// ============================================================
// Dinosaur vital stats struct
// ============================================================
USTRUCT(BlueprintType)
struct FDB_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.f;          // 0=starving, 100=full

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRadius = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WalkSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RunSpeed = 700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsCarnivore = true;
};

// ============================================================
// ADinosaurBase — base class for all dinosaur actors
// ============================================================
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ---- Overrides ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                             class AController* EventInstigator, AActor* DamageCauser) override;

    // ---- Species & Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDB_DinosaurSpecies Species = EDB_DinosaurSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FDB_DinosaurStats Stats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    EDB_BehaviourState CurrentBehaviour = EDB_BehaviourState::Idle;

    // ---- Patrol ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    float PatrolRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    float PatrolWaitTime = 3.f;

    // ---- Blueprint Events ----
    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnDinosaurDeath();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnTargetDetected(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnAttackLanded(AActor* Target, float DamageDealt);

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void SetBehaviourState(EDB_BehaviourState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    AActor* FindNearestThreat() const;

protected:
    // ---- Internal AI ----
    void TickPatrol(float DeltaTime);
    void TickHunting(float DeltaTime);
    void TickIdle(float DeltaTime);
    void MoveToPatrolPoint();
    void PerformAttack(AActor* Target);
    void Die();

    UPROPERTY()
    FVector PatrolOrigin;

    UPROPERTY()
    FVector CurrentPatrolTarget;

    UPROPERTY()
    float PatrolWaitTimer = 0.f;

    UPROPERTY()
    bool bWaitingAtPatrolPoint = false;

    UPROPERTY()
    TWeakObjectPtr<AActor> CurrentTarget;

    UPROPERTY()
    float AttackCooldown = 0.f;

    static constexpr float AttackCooldownDuration = 1.5f;
};
