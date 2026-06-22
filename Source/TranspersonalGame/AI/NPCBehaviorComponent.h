// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260622_009
// Modular NPC behavior component: patrol, alert, attack state machine
// Attaches to any APawn/ACharacter subclass (dinosaurs, human NPCs)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "NPCBehaviorComponent.generated.h"

// ─── Enums at global scope (UHT rule) ────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Generic     UMETA(DisplayName = "Generic")
};

// ─── Structs at global scope ──────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_PatrolConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaypointAcceptanceRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float IdleWaitTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    int32 NumWaypoints = 6;
};

USTRUCT(BlueprintType)
struct FNPC_CombatConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float DetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float ChaseSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    float FleeHealthThreshold = 0.2f;
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float TimeSinceLastSighting = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bHasSeenPlayer = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;  // 0.0 = no threat, 1.0 = maximum threat
};

// ─── Component ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
       DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Configuration ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FNPC_PatrolConfig PatrolConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
    FNPC_CombatConfig CombatConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FVector HomeLocation = FVector::ZeroVector;

    // ── Runtime State ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float CurrentHealth = 500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FNPC_MemoryEntry Memory;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float TimeSinceLastAttack = 0.0f;

    // ── Blueprint API ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
    float ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsHostile() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void UpdatePlayerSighting(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ClearPlayerMemory();

    UFUNCTION(BlueprintPure, Category = "NPC|Patrol")
    FVector GetNextPatrolWaypoint() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvancePatrolWaypoint();

    // ── Delegates ─────────────────────────────────────────────────────────
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPC_StateChangedDelegate,
        ENPC_BehaviorState, OldState, ENPC_BehaviorState, NewState);

    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_StateChangedDelegate OnStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_DeathDelegate, AActor*, Killer);

    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_DeathDelegate OnNPCDeath;

private:
    // ── Internal state ────────────────────────────────────────────────────
    UPROPERTY()
    TArray<FVector> PatrolWaypoints;

    int32 CurrentWaypointIndex = 0;
    float IdleTimer = 0.0f;
    bool bPatrolWaypointsGenerated = false;

    // ── Internal helpers ──────────────────────────────────────────────────
    void GeneratePatrolWaypoints();
    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickFlee(float DeltaTime);
    void UpdateMemoryDecay(float DeltaTime);
    bool CanSeePlayer(AActor* Player) const;
    float GetDistanceToPlayer() const;
    AActor* FindPlayerActor() const;
};
