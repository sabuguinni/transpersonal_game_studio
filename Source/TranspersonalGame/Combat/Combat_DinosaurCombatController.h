#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "SharedTypes.h"
#include "Combat_DinosaurCombatController.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DinosaurCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float FleeHealthThreshold = 0.3f;

    FCombat_DinosaurCombatStats()
    {
        AttackDamage = 50.0f;
        AttackRange = 300.0f;
        DetectionRadius = 2000.0f;
        MovementSpeed = 400.0f;
        AttackCooldown = 2.0f;
        FleeHealthThreshold = 0.3f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinosaurCombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_DinosaurCombatState GetCombatState() const { return CurrentCombatState; }

    // Player Detection
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool DetectPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInAttackRange() const;

    // Movement
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void MoveToTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FleeFromTarget();

protected:
    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISenseConfig_Sight* SightConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // Combat Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_DinosaurCombatStats CombatStats;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    ECombat_DinosaurCombatState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    FVector PatrolCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PatrolRadius = 1000.0f;

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal Functions
    void InitializePerception();
    void InitializeBehaviorTree();
    FVector GetRandomPatrolPoint();
    bool LineOfSightToTarget() const;
    float GetDistanceToTarget() const;
    void UpdateBlackboard();
};