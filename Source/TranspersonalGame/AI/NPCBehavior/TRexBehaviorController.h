#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TRexBehaviorController.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FNPC_TRexSenses
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float SightAngleDegrees = 120.0f;
};

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ATRexBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexBehaviorController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void SetBehaviorState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void SetPatrolTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    float GetDistanceToPlayer() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    FNPC_TRexSenses Senses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    ENPC_TRexState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Patrol")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackCooldown = 2.5f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Combat")
    float LastAttackTime;

private:
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateChaseBehavior(float DeltaTime);
    void UpdateAttackBehavior(float DeltaTime);
    void ScanForPlayer();
    APawn* GetPlayerPawn() const;
    FVector GetPatrolOrigin() const;

    FVector PatrolOrigin;
    float StateTimer;
};
