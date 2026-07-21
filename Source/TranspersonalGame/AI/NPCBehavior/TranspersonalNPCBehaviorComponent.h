#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "TranspersonalNPCBehaviorComponent.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1: no nesting inside UCLASS)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Alerted         UMETA(DisplayName = "Alerted"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatResponse : uint8
{
    Ignore          UMETA(DisplayName = "Ignore"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Flee            UMETA(DisplayName = "Flee"),
    Alert           UMETA(DisplayName = "Alert"),
    Attack          UMETA(DisplayName = "Attack")
};

UENUM(BlueprintType)
enum class ENPC_DailyRoutinePhase : uint8
{
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Socialising     UMETA(DisplayName = "Socialising"),
    Guarding        UMETA(DisplayName = "Guarding"),
    Resting         UMETA(DisplayName = "Resting"),
    Hunting         UMETA(DisplayName = "Hunting")
};

// ============================================================
// STRUCTS — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = true;
};

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UTranspersonalNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTranspersonalNPCBehaviorComponent();

    // ---- Lifecycle ----
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- State ----
    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    ENPC_DailyRoutinePhase CurrentRoutinePhase = ENPC_DailyRoutinePhase::Foraging;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    float ThreatAwareness = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    float AlertCooldown = 5.0f;

    // ---- Perception ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float HearingRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float FleeRadius = 800.0f;

    // ---- Memory ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayRate = 0.05f;

    // ---- Patrol ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius = 2000.0f;

    // ---- Behavior Tree ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
    UBehaviorTree* BehaviorTreeAsset = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|AI")
    UBlackboardComponent* BlackboardComp = nullptr;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel, FVector LastKnownLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_ThreatResponse EvaluateThreatResponse(float IncomingThreatLevel) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AdvancePatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetCurrentPatrolTarget() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    float GetHighestActiveThreat() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool HasActiveThreat() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdateDailyRoutine(float GameTimeHours);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void GeneratePatrolPointsAroundOrigin(FVector Origin, int32 NumPoints);

private:
    void TickMemoryDecay(float DeltaTime);
    void UpdateThreatAwareness(float DeltaTime);
    void SyncStateToBlackboard();

    float TimeSinceLastAlert = 0.0f;
    FVector SpawnOrigin = FVector::ZeroVector;
};
