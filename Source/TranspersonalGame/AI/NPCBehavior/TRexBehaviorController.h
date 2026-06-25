#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TRexBehaviorController.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrolling   UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing      UMETA(DisplayName = "Chasing"),
    Attacking    UMETA(DisplayName = "Attacking"),
    Resting      UMETA(DisplayName = "Resting")
};

UCLASS(ClassGroup = "NPCBehavior", meta = (DisplayName = "T-Rex Behavior Controller"))
class TRANSPERSONALGAME_API ATRexBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexBehaviorController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void ChaseTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    float GetDetectionRange() const { return DetectionRange; }

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    float GetAttackRange() const { return AttackRange; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float ChaseSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Config")
    float PatrolSpeed = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|TRex|State", meta = (AllowPrivateAccess = "true"))
    ENPC_TRexState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|TRex|State", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AActor> CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex|Patrol")
    TArray<TObjectPtr<AActor>> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|TRex|Patrol", meta = (AllowPrivateAccess = "true"))
    int32 CurrentWaypointIndex = 0;

private:
    void UpdateBehavior(float DeltaTime);
    void ScanForPlayer();
    void MoveToNextWaypoint();
    bool IsPlayerInRange(float Range) const;
    float TimeSinceLastAttack = 0.0f;
    float AttackCooldown = 2.5f;
};
