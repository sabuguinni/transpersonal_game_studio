#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Combat_DinosaurAIController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISightConfig* SightConfig;

    // Combat states
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float CombatRange;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float DetectionRange;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float AttackCooldown;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float LastAttackTime;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool DetectPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack();

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

private:
    AActor* CurrentTarget;
    FTimerHandle CombatCheckTimer;
    
    void CombatTick();
    void SetupPerception();
};