#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "NPC_TRexBehaviorController.generated.h"

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
struct FNPC_TRexBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float HuntingSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float Health = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    float Aggression = 0.8f;

    FNPC_TRexBehaviorData()
    {
        PatrolRadius = 5000.0f;
        DetectionRange = 3000.0f;
        AttackRange = 300.0f;
        MovementSpeed = 600.0f;
        HuntingSpeed = 900.0f;
        Health = 1000.0f;
        Hunger = 50.0f;
        Aggression = 0.8f;
    }
};

/**
 * T-Rex AI Controller with realistic predator behavior
 * Handles patrol, hunting, and territorial behavior patterns
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_TRexBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_TRexBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISightConfig* SightConfig;

    // Behavior Tree Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBlackboardComponent* BlackboardComponent;

    // T-Rex Behavior Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
    FNPC_TRexBehaviorData BehaviorData;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex State")
    ENPC_TRexState CurrentState;

    // Target and Patrol
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Targets")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Patrol")
    FVector HomeLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Patrol")
    FVector CurrentPatrolTarget;

    // Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Timers")
    float StateTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Timers")
    float LastAttackTime;

public:
    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void SetState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StartAttack();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    bool IsInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    bool IsInDetectionRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    FVector GetRandomPatrolPoint() const;

    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

protected:
    // Internal Behavior Logic
    void UpdateBehavior(float DeltaTime);
    void UpdatePatrol(float DeltaTime);
    void UpdateHunting(float DeltaTime);
    void UpdateAttacking(float DeltaTime);
    void UpdateFeeding(float DeltaTime);

    // Utility Functions
    float GetDistanceToTarget(AActor* Target) const;
    bool CanSeeTarget(AActor* Target) const;
    void SetMovementSpeed(float Speed);
    
    // Blackboard Keys
    static const FName TargetActorKey;
    static const FName PatrolLocationKey;
    static const FName StateKey;
    static const FName HomeLocationKey;
};