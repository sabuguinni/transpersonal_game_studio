#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "NPCBehaviorTree.generated.h"

// ============================================================
// NPC Behavior Tree System — Agent #11 NPC Behavior
// Defines Blackboard keys and AI Controller for NPC agents
// Supports: Patrol, Forage, Rest, Flee, Alert, Interact states
// ============================================================

// Blackboard key names used by all NPC behavior trees
namespace NPC_BlackboardKeys
{
    static const FName TargetLocation(TEXT("TargetLocation"));
    static const FName HomeLocation(TEXT("HomeLocation"));
    static const FName ThreatActor(TEXT("ThreatActor"));
    static const FName ThreatLevel(TEXT("ThreatLevel"));
    static const FName BehaviorState(TEXT("BehaviorState"));
    static const FName PatrolIndex(TEXT("PatrolIndex"));
    static const FName TimeOfDay(TEXT("TimeOfDay"));
    static const FName IsHungry(TEXT("IsHungry"));
    static const FName IsTired(TEXT("IsTired"));
    static const FName LastKnownThreatLocation(TEXT("LastKnownThreatLocation"));
}

// ============================================================
// NPC Personality Traits — drives decision weights
// ============================================================
UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Cautious        UMETA(DisplayName = "Cautious"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Curious         UMETA(DisplayName = "Curious"),
    Timid           UMETA(DisplayName = "Timid"),
    Territorial     UMETA(DisplayName = "Territorial")
};

// ============================================================
// NPC Behavior State — current FSM state
// ============================================================
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Forage          UMETA(DisplayName = "Forage"),
    Rest            UMETA(DisplayName = "Rest"),
    Flee            UMETA(DisplayName = "Flee"),
    Alert           UMETA(DisplayName = "Alert"),
    Interact        UMETA(DisplayName = "Interact"),
    Dead            UMETA(DisplayName = "Dead")
};

// ============================================================
// Daily Routine Entry — time-of-day scheduled behavior
// ============================================================
USTRUCT(BlueprintType)
struct FNPC_DailyRoutineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float StartHour = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float EndHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState ScheduledState = ENPC_BehaviorState::Rest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    FString ActivityDescription;
};

// ============================================================
// NPC Threat Memory Entry — tracks perceived threats
// ============================================================
USTRUCT(BlueprintType)
struct FNPC_ThreatMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    TWeakObjectPtr<AActor> ThreatActor;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimePerceived = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float DecayRate = 0.1f;
};

// ============================================================
// NPC Patrol Waypoint — defines a point on a patrol route
// ============================================================
USTRUCT(BlueprintType)
struct FNPC_PatrolWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = true;
};

// ============================================================
// ANPCBehaviorController — AI Controller for NPC agents
// Manages BehaviorTree execution and Blackboard updates
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = "NPC")
class TRANSPERSONALGAME_API ANPCBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ANPCBehaviorController();

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Behavior Tree ----
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|AI",
        meta = (AllowPrivateAccess = "true"))
    UBehaviorTreeComponent* BehaviorTreeComp;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|AI",
        meta = (AllowPrivateAccess = "true"))
    UBlackboardComponent* BlackboardComp;

    // ---- Personality ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Personality")
    ENPC_PersonalityTrait PersonalityTrait = ENPC_PersonalityTrait::Cautious;

    // ---- State ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentBehaviorState = ENPC_BehaviorState::Idle;

    // ---- Patrol ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolWaypoint> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius = 2000.0f;

    // ---- Daily Routine ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_DailyRoutineEntry> DailyRoutine;

    // ---- Threat Memory ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_ThreatMemoryEntry> ThreatMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatMemoryDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float FleeThreshold = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float AlertThreshold = 0.3f;

    // ---- Perception ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float SightRange = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float HearingRange = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float SightAngleDegrees = 120.0f;

    // ---- Blueprint-callable API ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ClearThreatMemory();

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    float GetHighestThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    ENPC_BehaviorState GetScheduledBehaviorForHour(float CurrentHour) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdateBlackboardFromState();

protected:
    void InitializeDefaultDailyRoutine();
    void UpdateThreatMemoryDecay(float DeltaTime);
    void EvaluateThreatResponse();
    bool HasLineOfSightToActor(AActor* Target) const;

private:
    float TimeSinceLastStateEval = 0.0f;
    static constexpr float StateEvalInterval = 0.5f;
};
