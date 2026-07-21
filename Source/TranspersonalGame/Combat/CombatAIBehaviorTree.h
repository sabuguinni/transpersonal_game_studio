#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "CombatAIBehaviorTree.generated.h"

/**
 * Combat AI Behavior Tree Manager
 * Manages tactical combat behaviors for dinosaurs and enemies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_AIBehaviorTree : public UObject
{
    GENERATED_BODY()

public:
    UCombat_AIBehaviorTree();

    // Combat behavior states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_AIState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TacticalIntelligence;

    // Combat target tracking
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* PrimaryTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TArray<AActor*> SecondaryTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange;

    // Tactical behavior functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeCombatBehavior(APawn* OwnerPawn);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPrimaryTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetTacticalPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteCombatAction(ECombat_ActionType ActionType);

private:
    UPROPERTY()
    APawn* OwnerPawn;

    UPROPERTY()
    UBlackboardComponent* BlackboardComp;

    float LastCombatUpdateTime;
    float CombatStateTimer;

    // Internal tactical calculations
    void CalculateAggressionLevel();
    void CalculateFearLevel();
    void UpdateTargetPriorities();
    bool IsTargetVisible(AActor* Target);
    float GetDistanceToTarget(AActor* Target);
};