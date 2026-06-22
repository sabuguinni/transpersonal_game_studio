#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DinosaurCombatAI.generated.h"

// Combat state enum for dinosaur AI
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

// Dinosaur species type for combat behaviour variation
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Triceratops         UMETA(DisplayName = "Triceratops")
};

// Combat attack type
UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    TailSwipe   UMETA(DisplayName = "Tail Swipe"),
    Charge      UMETA(DisplayName = "Charge"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Dive        UMETA(DisplayName = "Dive")
};

// Patrol waypoint data
USTRUCT(BlueprintType)
struct FCombat_PatrolWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    float WaitTimeAtPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    bool bIsAlertPoint;

    FCombat_PatrolWaypoint()
        : Location(FVector::ZeroVector)
        , WaitTimeAtPoint(2.0f)
        , bIsAlertPoint(false)
    {}
};

// Combat stats for a dinosaur
USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FleeHealthThreshold;

    FCombat_DinoStats()
        : MaxHealth(500.0f)
        , CurrentHealth(500.0f)
        , AttackDamage(75.0f)
        , AttackRange(200.0f)
        , DetectionRadius(1500.0f)
        , ChaseSpeed(600.0f)
        , PatrolSpeed(200.0f)
        , AttackCooldown(2.0f)
        , FleeHealthThreshold(0.15f)
    {}
};

/**
 * ADinosaurCombatAI — Base combat AI actor for all dinosaur enemies.
 * Implements state machine: Idle → Patrol → Investigate → Chase → Attack → Flee.
 * Each species has unique combat parameters and attack patterns.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Species & State ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    // ── Patrol ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<FCombat_PatrolWaypoint> PatrolWaypoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Patrol")
    int32 CurrentWaypointIndex;

    // ── Detection ──
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Detection")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Detection")
    USphereComponent* AttackRangeSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Mesh")
    UStaticMeshComponent* BodyMesh;

    // ── Target ──
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Target")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Target")
    float TimeSinceLastAttack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Target")
    float TimeInCurrentState;

    // ── Combat Functions ──
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeCombatDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitialiseSpeciesStats(ECombat_DinoSpecies InSpecies);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddPatrolWaypoint(FVector Location, float WaitTime = 2.0f, bool bAlertPoint = false);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
    void OnDeath();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
    void OnDetectPlayer(AActor* Player);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
    void OnLoseTarget();

private:
    void UpdateIdleState(float DeltaTime);
    void UpdatePatrolState(float DeltaTime);
    void UpdateChaseState(float DeltaTime);
    void UpdateAttackState(float DeltaTime);
    void UpdateFleeState(float DeltaTime);

    void MoveTowardsLocation(FVector TargetLocation, float Speed, float DeltaTime);
    bool IsPlayerInRange(float Range) const;
    AActor* FindNearestPlayer() const;

    UFUNCTION()
    void OnDetectionSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAttackRangeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
