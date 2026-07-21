// DinosaurCombatAI.h
// Combat & Enemy AI Agent #12 — Transpersonal Game Studio
// Dinosaur tactical combat AI: T-Rex pursuit/patrol, Raptor pack flanking
// Cycle: PROD_CYCLE_AUTO_20260701_004

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Navigation/PathFollowingComponent.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// ENUMS — all prefixed Combat_ to avoid global name conflicts
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Curious     UMETA(DisplayName = "Curious"),
    Agitated    UMETA(DisplayName = "Agitated"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Lethal      UMETA(DisplayName = "Lethal")
};

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Feeding     UMETA(DisplayName = "Feeding")
};

UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha Distractor"),
    LeftFlanker UMETA(DisplayName = "Left Flanker"),
    RightFlanker UMETA(DisplayName = "Right Flanker"),
    Ambusher    UMETA(DisplayName = "Ambusher"),
    Scout       UMETA(DisplayName = "Scout")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MoveSpeedPatrol = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MoveSpeedChase = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AggroRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 2.5f;
};

USTRUCT(BlueprintType)
struct FCombat_PatrolWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    float WaitTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    bool bLookAround = true;
};

// ============================================================
// MAIN COMBAT AI COMPONENT
// ============================================================

UCLASS(ClassGroup = (CombatAI), meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Combat AI")
class TRANSPERSONALGAME_API UCombat_DinosaurAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Species & State ──────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TyrannosaurusRex;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    // ── Stats ────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    // ── Patrol ───────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<FCombat_PatrolWaypoint> PatrolWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    bool bLoopPatrol = true;

    // ── Target ───────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Target")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Target")
    float DistanceToTarget = 0.0f;

    // ── Functions ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void TakeDamage_Combat(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void TransitionToState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ExecuteAttack();

private:
    float AttackCooldownTimer = 0.0f;
    int32 CurrentWaypointIndex = 0;
    float WaypointWaitTimer = 0.0f;
    bool bWaitingAtWaypoint = false;

    void UpdatePatrol(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void EvaluateThreat();
};

// ============================================================
// RAPTOR PACK COORDINATOR
// ============================================================

UCLASS(ClassGroup = (CombatAI), meta = (BlueprintSpawnableComponent), DisplayName = "Raptor Pack AI")
class TRANSPERSONALGAME_API UCombat_RaptorPackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_RaptorPackComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    ECombat_RaptorRole PackRole = ECombat_RaptorRole::Alpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float FlankingOffset = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float CoordinationRadius = 1200.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    AActor* SharedTarget = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void SetSharedTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    FVector CalculateFlankPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void BroadcastTargetToPackMembers(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    bool IsFlankingPositionReached() const;

private:
    FVector AssignedFlankPosition = FVector::ZeroVector;
    bool bPositionAssigned = false;
};
