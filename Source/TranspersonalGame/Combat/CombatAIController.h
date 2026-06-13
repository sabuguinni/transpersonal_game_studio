#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "CombatAIController.generated.h"

class APawn;
class UBehaviorTree;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree Asset
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    // Combat Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AIState CurrentAIState;

    // Perception Configuration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
    class UAISenseConfig_Hearing* HearingConfig;

public:
    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetCombatTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInCombat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FleeFromThreat();

    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    ECombat_AIState GetAIState() const;

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat Logic
    void UpdateCombatBehavior();
    void EvaluateThreatLevel();
    bool ShouldFlee() const;
    bool ShouldAttack() const;
    bool ShouldInvestigate() const;

private:
    // Internal State
    AActor* CurrentTarget;
    float LastCombatTime;
    bool bInCombat;
    float ThreatLevel;
};