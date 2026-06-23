#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRexBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FNPC_TRexSensoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float SmellRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    bool bPlayerDetected = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FNPC_TRexCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackDamage = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChaseSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float PatrolSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float MaxHealth = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float CurrentHealth = 2000.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTRexBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    ENPC_TRexState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    FNPC_TRexSensoryData SensoryData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    FNPC_TRexCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    FVector PatrolOrigin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolWaitTime = 3.0f;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void SetState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool IsPlayerInSightRange() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void TakeDamage_TRex(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    bool IsAlive() const { return CombatStats.CurrentHealth > 0.0f; }

    UFUNCTION(BlueprintPure, Category = "TRex|Behavior")
    FString GetStateAsString() const;

private:
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateChaseBehavior(float DeltaTime);
    void UpdateAttackBehavior(float DeltaTime);
    void UpdateSensoryDetection();
    FVector GetRandomPatrolPoint() const;

    float TimeSinceLastAttack = 0.0f;
    float PatrolWaitTimer = 0.0f;
    bool bWaitingAtPatrolPoint = false;
    FVector CurrentPatrolTarget = FVector::ZeroVector;
    AActor* CachedPlayerActor = nullptr;
};
