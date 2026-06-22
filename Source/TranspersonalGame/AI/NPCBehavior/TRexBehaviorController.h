// TRexBehaviorController.h
// NPC Behavior Agent #11 — T-Rex AI Controller with patrol, chase, and attack states
// PROD_CYCLE_AUTO_20260622_008

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "TRexBehaviorController.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Roaring     UMETA(DisplayName = "Roaring"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FNPC_TRexPerceptionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|TRex")
    AActor* LastKnownTarget = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|TRex")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|TRex")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|TRex")
    bool bTargetInSight = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|TRex")
    bool bHeardSomething = false;
};

UCLASS(ClassGroup = "TranspersonalGame|AI", meta = (DisplayName = "T-Rex Behavior Controller"))
class TRANSPERSONALGAME_API ATRexBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexBehaviorController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // --- State Machine ---
    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void SetTRexState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|TRex")
    ENPC_TRexState GetTRexState() const { return CurrentState; }

    // --- Patrol ---
    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void SetPatrolWaypoints(const TArray<AActor*>& Waypoints);

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void MoveToNextWaypoint();

    // --- Combat ---
    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void StartChasing(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void RoarAlert();

    // --- Perception callback ---
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float ChaseDetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float ChaseSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float WaypointAcceptanceRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float InvestigateTimeout = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float SightRadius = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float HearingRange = 2000.0f;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|TRex|State")
    ENPC_TRexState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|TRex|State")
    FNPC_TRexPerceptionData PerceptionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|TRex|Patrol")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|TRex|Patrol")
    int32 CurrentWaypointIndex;

    UPROPERTY(VisibleAnywhere, Category = "NPC|TRex|Components")
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere, Category = "NPC|TRex|Components")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, Category = "NPC|TRex|Components")
    UAISenseConfig_Hearing* HearingConfig;

private:
    float StateTimer;
    float AttackCooldownTimer;
    bool bAttackOnCooldown;

    void UpdateIdleState(float DeltaTime);
    void UpdatePatrolState(float DeltaTime);
    void UpdateInvestigateState(float DeltaTime);
    void UpdateChaseState(float DeltaTime);
    void UpdateAttackState(float DeltaTime);

    float GetDistanceToTarget() const;
    bool IsTargetVisible() const;
    void SetMovementSpeed(float Speed);
};
