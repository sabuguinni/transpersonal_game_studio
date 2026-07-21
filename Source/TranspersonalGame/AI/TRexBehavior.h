#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRexBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Roar        UMETA(DisplayName = "Roar")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChaseSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarCooldown = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float SightAngle = 60.0f;
};

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent), DisplayName = "T-Rex Behavior Component")
class TRANSPERSONALGAME_API UTRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTRexBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State Machine ---
    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    void SetState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintPure, Category = "TRex|State")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    // --- Patrol ---
    UFUNCTION(BlueprintCallable, Category = "TRex|Patrol")
    void PickNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "TRex|Patrol")
    bool HasReachedPatrolPoint() const;

    // --- Detection ---
    UFUNCTION(BlueprintCallable, Category = "TRex|Detection")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Detection")
    bool CanHearPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Detection")
    float GetDistanceToPlayer() const;

    // --- Combat ---
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    bool IsInAttackRange() const;

    // --- Roar ---
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void TriggerRoar();

    UFUNCTION(BlueprintPure, Category = "TRex|Behavior")
    bool CanRoar() const;

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    FNPC_TRexConfig Config;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    ENPC_TRexState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Patrol")
    FVector CurrentPatrolTarget;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Patrol")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Combat")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackCooldown = 2.0f;

private:
    float LastRoarTime;
    float StateTimer;

    void UpdatePatrol(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateInvestigate(float DeltaTime);

    AActor* GetPlayerActor() const;
};
