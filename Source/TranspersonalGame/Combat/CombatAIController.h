#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "CombatAIController.generated.h"

class UCombat_TacticalBehaviorComponent;
class UCombat_PackCoordinationComponent;
class APawn;

/**
 * Advanced AI Controller for tactical combat behavior
 * Handles perception, decision-making, and pack coordination
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

	// Core AI Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	class UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	class UBlackboardComponent* BlackboardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	UCombat_TacticalBehaviorComponent* TacticalBehaviorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	UCombat_PackCoordinationComponent* PackCoordinationComponent;

	// AI Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
	class UBehaviorTree* BehaviorTreeAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
	class UBlackboardData* BlackboardAsset;

	// Combat AI Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
	ECombat_AIPersonality AIPersonality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
	ECombat_TacticalRole TacticalRole;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
	float AggressionLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
	float CautiousLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
	float PackLoyalty;

	// Perception Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float SightRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float LoseSightRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float PeripheralVisionAngleDegrees;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float HearingRange;

	// Combat State
	UPROPERTY(BlueprintReadOnly, Category = "Combat State")
	ECombat_AIState CurrentAIState;

	UPROPERTY(BlueprintReadOnly, Category = "Combat State")
	AActor* CurrentTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Combat State")
	AActor* PackLeader;

	UPROPERTY(BlueprintReadOnly, Category = "Combat State")
	TArray<ACombatAIController*> PackMembers;

public:
	// AI State Management
	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	void SetAIState(ECombat_AIState NewState);

	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	ECombat_AIState GetAIState() const { return CurrentAIState; }

	// Target Management
	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	void SetTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	// Pack Management
	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	void JoinPack(ACombatAIController* Leader);

	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	void LeavePack();

	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	bool IsPackLeader() const;

	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	void AddPackMember(ACombatAIController* Member);

	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	void RemovePackMember(ACombatAIController* Member);

	// Combat Decision Making
	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	bool ShouldEngageTarget(AActor* PotentialTarget) const;

	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	FVector GetTacticalPosition(AActor* Target, ECombat_TacticalRole Role) const;

	UFUNCTION(BlueprintCallable, Category = "Combat AI")
	bool CanExecuteTactic(ECombat_TacticType Tactic) const;

protected:
	// Perception Callbacks
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// AI State Transitions
	void UpdateAIState(float DeltaTime);
	void TransitionToState(ECombat_AIState NewState);

	// Combat Logic
	void UpdateCombatBehavior(float DeltaTime);
	void EvaluateThreats();
	void CoordinateWithPack();

private:
	// Internal state tracking
	float StateTimer;
	float LastTargetUpdateTime;
	FVector LastKnownTargetLocation;
	bool bHasValidTarget;

	// Perception setup
	void SetupPerception();
	void ConfigureSightSense();
	void ConfigureHearingSense();
};