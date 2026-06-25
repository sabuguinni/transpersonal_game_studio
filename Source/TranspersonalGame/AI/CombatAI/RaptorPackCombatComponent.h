#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "RaptorPackCombatComponent.generated.h"

// ============================================================
// Raptor Pack Combat Role — each raptor has a fixed role
// ============================================================
UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Distractor   UMETA(DisplayName = "Distractor"),   // Charges front, draws attention
    FlankLeft    UMETA(DisplayName = "FlankLeft"),    // Circles to player's left
    FlankRight   UMETA(DisplayName = "FlankRight"),   // Circles to player's right
    Ambush       UMETA(DisplayName = "Ambush"),       // Waits hidden, strikes when player retreats
};

// ============================================================
// Raptor Combat State
// ============================================================
UENUM(BlueprintType)
enum class ECombat_RaptorState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Stalking     UMETA(DisplayName = "Stalking"),     // Moving into position, low aggro
    Flanking     UMETA(DisplayName = "Flanking"),     // Executing flank maneuver
    Distracting  UMETA(DisplayName = "Distracting"), // Frontal feint
    Attacking    UMETA(DisplayName = "Attacking"),   // Committed attack
    Retreating   UMETA(DisplayName = "Retreating"),  // Backing off after hit
    Dead         UMETA(DisplayName = "Dead"),
};

// ============================================================
// Shared pack blackboard data (all raptors read this)
// ============================================================
USTRUCT(BlueprintType)
struct FCombat_RaptorPackBlackboard
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    bool bPlayerDetected = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    bool bDistractorEngaged = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    float TimeSincePlayerSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    int32 ActiveAttackers = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    float PackHungerLevel = 0.5f;  // 0=not hungry, 1=starving (affects aggression)
};

// ============================================================
// Per-raptor combat stats
// ============================================================
USTRUCT(BlueprintType)
struct FCombat_RaptorStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FlankingSpeed = 650.0f;   // cm/s while flanking

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float StalkingSpeed = 300.0f;   // cm/s while stalking

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 120.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Stats")
    float CurrentHealth = 120.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Stats")
    float LastAttackTime = 0.0f;
};

// ============================================================
// URaptorPackCombatComponent
// Attach to each Raptor SkeletalMeshActor.
// All 3 raptors share a pointer to the same FCombat_RaptorPackBlackboard.
// ============================================================
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API URaptorPackCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URaptorPackCombatComponent();

    // ---- Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    ECombat_RaptorRole PackRole = ECombat_RaptorRole::Distractor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_RaptorStats Stats;

    // Patrol waypoints for this raptor's role
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<AActor*> PatrolWaypoints;

    // Shared blackboard — set this on all 3 raptors to the same object
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FCombat_RaptorPackBlackboard PackBlackboard;

    // ---- State ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_RaptorState CurrentState = ECombat_RaptorState::Idle;

    // ---- Blueprint Events ----
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnStateChanged(ECombat_RaptorState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnAttackPlayer(float Damage);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnFlankPositionReached();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnPackmateEngaged(AActor* Packmate);

    // ---- Callable Functions ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void SetPackBlackboard(FCombat_RaptorPackBlackboard& SharedBlackboard);

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void ForceState(ECombat_RaptorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    ECombat_RaptorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|Combat")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Combat")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Combat")
    bool IsFlankPositionClear() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Combat")
    void TakeDamage_Raptor(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat|Combat")
    bool IsAlive() const { return Stats.CurrentHealth > 0.0f; }

    // ---- UActorComponent Overrides ----
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    AActor* CachedPlayer = nullptr;

    int32 CurrentWaypointIndex = 0;
    float WaypointWaitTimer = 0.0f;
    float AITickAccumulator = 0.0f;
    static constexpr float AI_TICK_RATE = 0.1f;  // 10Hz

    void UpdateStateMachine(float DeltaTime);
    void UpdateIdle(float DeltaTime);
    void UpdateStalking(float DeltaTime);
    void UpdateFlanking(float DeltaTime);
    void UpdateDistracting(float DeltaTime);
    void UpdateAttacking(float DeltaTime);
    void UpdateRetreating(float DeltaTime);

    void TransitionToState(ECombat_RaptorState NewState);
    bool IsPlayerInRange(float Range) const;
    FVector GetFlankTargetPosition() const;
    void ExecuteAttack();
    void ApplyFearToPlayer(float FearAmount);
};
