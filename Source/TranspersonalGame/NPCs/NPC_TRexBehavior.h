#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "NPC_TRexBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

USTRUCT(BlueprintType)
struct FNPC_TRexStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float Health = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float Aggression = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float Fear = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float Energy = 100.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TRexBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TRexBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core behavior functions
    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void SetState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    // State-specific behaviors
    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void HandleIdleBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void HandlePatrolBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void HandleHuntingBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void HandleAttackingBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void HandleFeedingBehavior(float DeltaTime);

    // Detection and targeting
    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    AActor* DetectNearbyTargets();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    bool IsPlayerInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    float GetDistanceToPlayer() const;

    // Movement and navigation
    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void MoveToLocation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void PatrolAroundTerritory();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    FVector GetRandomPatrolPoint() const;

    // Combat behaviors
    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void PerformBiteAttack();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void PerformTailSwipe();

    // Audio and visual feedback
    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void PlayRoarSound();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void PlayFootstepSound();

protected:
    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
    ENPC_TRexState CurrentState = ENPC_TRexState::Idle;

    // T-Rex stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    FNPC_TRexStats Stats;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float AttackCooldown = 3.0f;

    // Territory and navigation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
    FVector TerritoryCenter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
    FVector CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
    AActor* TargetActor;

    // Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
    float StateTimer = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
    float LastAttackTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
    float IdleTimer = 0.0f;

    // Behavior flags
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
    bool bIsMoving = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
    bool bCanAttack = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
    bool bHasTarget = false;

private:
    // Helper functions
    void InitializeTerritoryCenter();
    void UpdateStats(float DeltaTime);
    bool IsInTerritory(const FVector& Location) const;
    void RotateTowardsTarget(const FVector& TargetLocation, float DeltaTime);
};