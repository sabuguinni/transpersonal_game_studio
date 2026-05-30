#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "NPC_TRexBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrol      UMETA(DisplayName = "Patrolling"),
    Chase       UMETA(DisplayName = "Chasing Player"),
    Attack      UMETA(DisplayName = "Attacking"),
    Return      UMETA(DisplayName = "Returning to Patrol")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TRexBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float ChaseDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float AttackDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float ChaseSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float AttackCooldown = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float PatrolWaitTime = 5.0f;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex State")
    ENPC_TRexState CurrentState = ENPC_TRexState::Patrol;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex State")
    FVector HomeLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex State")
    FVector CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex State")
    float LastAttackTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex State")
    float PatrolTimer = 0.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void SetState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    APawn* FindNearestPlayer();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    FVector GeneratePatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void PerformAttack();

private:
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateChaseBehavior(float DeltaTime);
    void UpdateAttackBehavior(float DeltaTime);
    void UpdateReturnBehavior(float DeltaTime);

    void MoveToLocation(const FVector& TargetLocation, float Speed);
    bool IsAtLocation(const FVector& Location, float Tolerance = 200.0f) const;
    float GetDistanceToPlayer() const;

    APawn* CachedPlayer = nullptr;
    float PlayerCacheTime = 0.0f;
    const float PlayerCacheTimeout = 1.0f; // Cache player reference for 1 second
};