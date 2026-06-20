#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// NPC Behavior Component — Agent #11 NPC Behavior
// Drives daily routines, memory, threat response, and patrol
// for all non-player characters (human NPCs and dinosaur pawns)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Alert           UMETA(DisplayName = "Alert"),
    Chase           UMETA(DisplayName = "Chase"),
    Attack          UMETA(DisplayName = "Attack"),
    Flee            UMETA(DisplayName = "Flee"),
    Rest            UMETA(DisplayName = "Rest"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel PerceivedThreat = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsHostile = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    int32 EncounterCount = 0;
};

USTRUCT(BlueprintType)
struct FNPC_PatrolData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> WaypointLocations;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaypointAcceptanceRadius = 150.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTimeAtWaypoint = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLoopPatrol = true;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- State Machine ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState NPC_CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState NPC_PreviousState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    float NPC_StateTimer = 0.0f;

    // ---- Perception ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float NPC_SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float NPC_HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float NPC_AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float NPC_ChaseRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float NPC_FleeHealthThreshold = 0.25f;

    // ---- Memory ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FNPC_MemoryEntry NPC_PlayerMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float NPC_MemoryDecayTime = 30.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool NPC_bHasPlayerInMemory = false;

    // ---- Patrol ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FNPC_PatrolData NPC_PatrolData;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    bool NPC_bIsWaitingAtWaypoint = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    float NPC_WaypointWaitTimer = 0.0f;

    // ---- Threat ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Threat")
    ENPC_ThreatLevel NPC_CurrentThreat = ENPC_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Threat")
    bool NPC_bIsPredator = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Threat")
    bool NPC_bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Threat")
    float NPC_AggressionLevel = 0.7f;

    // ---- Survival Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float NPC_Hunger = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float NPC_Health = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float NPC_HungerDecayRate = 0.005f;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void NPC_SetState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    ENPC_BehaviorState NPC_GetCurrentState() const { return NPC_CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void NPC_UpdatePlayerMemory(FVector PlayerLocation, ENPC_ThreatLevel Threat, bool bHostile);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void NPC_ClearPlayerMemory();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector NPC_GetNextPatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void NPC_AdvancePatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    ENPC_ThreatLevel NPC_EvaluateThreat(float DistanceToPlayer, float PlayerHealthNormalized, bool bPlayerIsExhausted) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Survival")
    bool NPC_IsHungry() const { return NPC_Hunger < 0.4f; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Survival")
    bool NPC_IsInjured() const { return NPC_Health < 0.3f; }

private:
    void UpdateMemoryDecay(float DeltaTime);
    void UpdateHunger(float DeltaTime);
    void UpdateStateTimer(float DeltaTime);
};
