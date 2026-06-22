#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "TRexBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FNPC_TRexPerceptionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "TRex|Perception")
    bool bPlayerDetectedBySight = false;

    UPROPERTY(BlueprintReadWrite, Category = "TRex|Perception")
    bool bPlayerDetectedBySound = false;

    UPROPERTY(BlueprintReadWrite, Category = "TRex|Perception")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "TRex|Perception")
    float TimeSinceLastDetection = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "TRex|Perception")
    float ThreatLevel = 0.0f;
};

/**
 * ATRexBehaviorController — AI Controller for T-Rex dinosaur
 * Implements: patrol (5000 unit radius), chase (3000 unit trigger), attack (300 unit range)
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRexBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexBehaviorController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // --- Behavior State ---
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void SetBehaviorState(ENPC_TRexState NewState);

    // --- Patrol ---
    UFUNCTION(BlueprintCallable, Category = "TRex|Patrol")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "TRex|Patrol")
    void MoveToNextWaypoint();

    UFUNCTION(BlueprintCallable, Category = "TRex|Patrol")
    void SetPatrolCenter(FVector Center, float Radius = 5000.0f);

    // --- Detection ---
    UFUNCTION(BlueprintCallable, Category = "TRex|Detection")
    bool IsPlayerInChaseRange() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Detection")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Detection")
    float GetDistanceToPlayer() const;

    // --- Combat ---
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void ChasePlayer();

    // --- Perception callbacks ---
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float ChaseRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float ChaseSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float SightAngle = 90.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    FNPC_TRexPerceptionData PerceptionData;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    FVector PatrolCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    float TimeSinceLastAttack = 0.0f;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Components")
    UAIPerceptionComponent* PerceptionComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Components")
    UAISenseConfig_Sight* SightConfig = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Components")
    UAISenseConfig_Hearing* HearingConfig = nullptr;

private:
    ENPC_TRexState CurrentState = ENPC_TRexState::Idle;

    TArray<FVector> PatrolWaypoints;
    APawn* PlayerPawn = nullptr;

    void GeneratePatrolWaypoints();
    void UpdateBehaviorLogic(float DeltaTime);
    APawn* FindPlayerPawn() const;
};
