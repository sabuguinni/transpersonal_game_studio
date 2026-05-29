#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Engine/Engine.h"
#include "Combat_DinosaurCombatController.generated.h"

class UBehaviorTree;
class APawn;
class AActor;

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

    // Combat AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Combat Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
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

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget = nullptr;

public:
    // Combat AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool DetectPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ChaseTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void PatrolArea();

protected:
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Combat timer handles
    FTimerHandle AttackTimerHandle;
    FTimerHandle DetectionTimerHandle;

    // Combat timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionInterval = 0.5f;
};