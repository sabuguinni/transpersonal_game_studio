#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TranspersonalGame/TranspersonalCharacter.h"
#include "Combat_TRexAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

USTRUCT(BlueprintType)
struct FCombat_TRexStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TerritoryRadius = 2000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TRexAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TRexAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    // Combat Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TRexStats TRexStats;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombat_TRexState CurrentState;

    // Target
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ATranspersonalCharacter* TargetPlayer;

    // Territory
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector TerritoryCenter;

public:
    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void DetectPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartHunting(ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ReturnToPatrol();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_TRexState NewState);

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsPlayerInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetRandomPatrolPoint() const;

    // Getters
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ATranspersonalCharacter* GetTargetPlayer() const { return TargetPlayer; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombat_TRexStats GetTRexStats() const { return TRexStats; }

private:
    // Internal timers
    float LastDetectionTime;
    float LastAttackTime;
    float PatrolWaitTime;

    // Constants
    static constexpr float DETECTION_INTERVAL = 0.5f;
    static constexpr float ATTACK_COOLDOWN = 2.0f;
    static constexpr float PATROL_WAIT_TIME = 3.0f;
};

#include "Combat_TRexAIController.generated.h"