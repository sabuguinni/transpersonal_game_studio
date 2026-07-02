// TRexBehavior.h
// NPC Behavior Agent #11 — T-Rex AI Controller
// T-Rex patrols 5000-unit radius, chases player within 3000 units, attacks within 300 units
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "TRexBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrol     UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase      UMETA(DisplayName = "Chase"),
    Attack     UMETA(DisplayName = "Attack"),
    Roar       UMETA(DisplayName = "Roar"),
    Idle       UMETA(DisplayName = "Idle")
};

USTRUCT(BlueprintType)
struct FNPC_TRexConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float ChaseRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackDamage = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChaseSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarCooldown = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float PatrolWaypointRadius = 500.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTRexBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    FNPC_TRexConfig Config;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    ENPC_TRexState CurrentState;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void SetState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool IsPlayerInChaseRange() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void TriggerRoar();

private:
    void UpdatePatrol(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void PickNewPatrolWaypoint();
    bool HasReachedWaypoint() const;

    FVector PatrolAnchor;
    FVector CurrentWaypoint;
    float TimeSinceLastRoar;
    float AttackCooldown;
    float StateTimer;

    UPROPERTY()
    AActor* PlayerActor;
};
