#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTags/GameplayTags.h"
#include "CombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

/**
 * Combat AI Controller for dinosaur enemies
 * Handles tactical combat behavior, threat assessment, and dynamic responses
 */
UCLASS()
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    // Blackboard
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardAsset* BlackboardAsset;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.7f;

    // Threat Assessment
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float ThreatLevel = 0.0f;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FGameplayTag CombatState;

    // Gameplay Tags for Combat States
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tags")
    FGameplayTag IdleState;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tags")
    FGameplayTag HuntingState;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tags")
    FGameplayTag AttackingState;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tags")
    FGameplayTag FleeingState;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tags")
    FGameplayTag PatrollingState;

public:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(FGameplayTag NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartFleeing();

    // Tactical Positioning
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector FindOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector FindFleePosition();

    // Group Behavior
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CallForBackup();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RespondToBackupCall(AActor* Caller);

private:
    void SetupPerception();
    void SetupBlackboard();
    void UpdateCombatLogic(float DeltaTime);
    
    // Internal state tracking
    float LastAttackTime = 0.0f;
    float AttackCooldown = 2.0f;
    bool bIsInCombat = false;
    
    // Threat tracking
    TMap<AActor*, float> ThreatMap;
    void UpdateThreatAssessment();
};