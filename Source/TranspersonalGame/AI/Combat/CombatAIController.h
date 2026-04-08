#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/StateTreeComponent.h"
#include "GameplayTags.h"
#include "CombatAIController.generated.h"

class UBlackboardComponent;
class UBehaviorTree;
class UStateTree;
class ACombatAICharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, FGameplayTag, PreviousState, FGameplayTag, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatLevelChanged, float, ThreatLevel);

/**
 * Enhanced AI Controller for combat scenarios in the prehistoric survival game.
 * Combines Behavior Trees for tactical decisions with State Trees for combat flow.
 * Designed to make the player feel like prey, not predator.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACombatAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Core AI Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
	class UStateTreeComponent* StateTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
	class UAIPerceptionComponent* PerceptionComponent;

	// Combat Behavior Trees
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat")
	class UBehaviorTree* PredatorBehaviorTree;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat")
	class UBehaviorTree* PreyBehaviorTree;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat")
	class UBehaviorTree* NeutralBehaviorTree;

	// Combat State Tree
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat")
	class UStateTree* CombatStateTree;

	// Blackboard Asset
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat")
	class UBlackboardAsset* CombatBlackboard;

public:
	// Combat State Management
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnCombatStateChanged OnCombatStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnThreatLevelChanged OnThreatLevelChanged;

	// Combat Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat|Parameters")
	float BaseThreatLevel = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat|Parameters")
	float AggressionLevel = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat|Parameters")
	float FearThreshold = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat|Parameters")
	float AttackRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat|Parameters")
	float FleeRange = 150.0f;

	// Tactical Positioning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat|Tactics")
	float OptimalCombatDistance = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat|Tactics")
	float CirclingRadius = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat|Tactics")
	float AmbushDistance = 800.0f;

	// Current Combat State
	UPROPERTY(BlueprintReadOnly, Category = "AI|Combat|State")
	FGameplayTag CurrentCombatState;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Combat|State")
	float CurrentThreatLevel;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Combat|State")
	AActor* CurrentTarget;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Combat|State")
	TArray<AActor*> PerceivedThreats;

	// Combat Functions
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void SetCombatState(FGameplayTag NewState);

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void UpdateThreatLevel(float NewThreatLevel);

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void SetPrimaryTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool ShouldEngageInCombat() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool ShouldFleeFromCombat() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	FVector GetOptimalAttackPosition() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	FVector GetFleePosition() const;

	// Perception Callbacks
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	// Internal state tracking
	float LastThreatAssessmentTime;
	float ThreatAssessmentInterval = 0.5f;
	
	FGameplayTag PreviousCombatState;
	
	// Cached references
	ACombatAICharacter* ControlledCharacter;

	// Internal functions
	void AssessThreatLevel();
	void UpdateCombatBehavior();
	void HandleStateTransition(FGameplayTag NewState);
	UBehaviorTree* SelectAppropriiateBehaviorTree() const;
};