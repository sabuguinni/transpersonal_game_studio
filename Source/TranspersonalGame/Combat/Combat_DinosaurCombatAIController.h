#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Engine/Engine.h"
#include "TranspersonalGame.h"
#include "Combat_DinosaurCombatAIController.generated.h"

class ATranspersonalCharacter;
class UBehaviorTree;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Combat AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* PerceptionComponent;

    // Combat Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* CombatBehaviorTree;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsAggressive = true;

public:
    // Combat AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool DetectPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartCombatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StopCombatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetCombatTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatState();

protected:
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Combat state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    bool bInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackCooldown = 2.0f;

private:
    void InitializePerception();
    void SetupBlackboardKeys();
    
    // Blackboard key names
    static const FName TargetActorKey;
    static const FName InCombatKey;
    static const FName LastKnownLocationKey;
    static const FName DistanceToTargetKey;
};