#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "../../SharedTypes.h"
#include "NPC_TRexBehavior.generated.h"

/**
 * T-Rex Behavior Component - Implements territorial patrol and hunting behavior
 * Manages T-Rex AI states: Patrol, Hunt, Attack, Return to Territory
 * Realistic predator behavior based on paleontological research
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TRexBehavior : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPC_TRexBehavior();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core behavior methods
	UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
	void InitializeBehavior(FVector TerritoryCenter, float TerritoryRadius);

	UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
	void UpdateBehaviorState();

	UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
	void SetTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
	void ClearTarget();

	// State queries
	UFUNCTION(BlueprintPure, Category = "T-Rex Behavior")
	ENPC_DinosaurState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "T-Rex Behavior")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	UFUNCTION(BlueprintPure, Category = "T-Rex Behavior")
	float GetDistanceToTarget() const;

	UFUNCTION(BlueprintPure, Category = "T-Rex Behavior")
	bool IsInTerritory() const;

protected:
	// Behavior state machine
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
	ENPC_DinosaurState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Behavior")
	ENPC_DinosaurState PreviousState;

	// Territory settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
	FVector TerritoryCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
	float TerritoryRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
	float PatrolRadius;

	// Detection ranges
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
	float DetectionRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
	float ChaseRange;

	// Current target and patrol
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
	AActor* CurrentTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	FVector CurrentPatrolPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	FVector NextPatrolPoint;

	// Timers and cooldowns
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timing")
	float StateChangeTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (AllowPrivateAccess = "true"))
	float PatrolPointReachThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing", meta = (AllowPrivateAccess = "true"))
	float AttackCooldown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timing")
	float LastAttackTime;

	// Movement speeds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float PatrolSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float ChaseSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float AttackSpeed;

private:
	// Internal behavior methods
	void HandlePatrolState();
	void HandleHuntState();
	void HandleAttackState();
	void HandleReturnState();

	// Utility methods
	FVector GenerateRandomPatrolPoint();
	AActor* FindNearestPlayer();
	bool CanAttackTarget() const;
	void MoveToLocation(FVector TargetLocation, float Speed);
	void RotateTowardsTarget(AActor* Target, float DeltaTime);

	// State transition methods
	void TransitionToState(ENPC_DinosaurState NewState);
	bool ShouldTransitionToHunt() const;
	bool ShouldTransitionToAttack() const;
	bool ShouldTransitionToReturn() const;
	bool ShouldTransitionToPatrol() const;
};