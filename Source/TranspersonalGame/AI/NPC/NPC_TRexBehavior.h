#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "NPC_TRexBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
	Patrolling		UMETA(DisplayName = "Patrolling"),
	Hunting			UMETA(DisplayName = "Hunting"), 
	Attacking		UMETA(DisplayName = "Attacking"),
	Feeding			UMETA(DisplayName = "Feeding"),
	Resting			UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FNPC_TRexStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
	float PatrolRadius = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
	float DetectionRange = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
	float AttackRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
	float MovementSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
	float HuntingSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
	float AttackDamage = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
	float RestDuration = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
	float FeedingDuration = 15.0f;

	FNPC_TRexStats()
	{
		PatrolRadius = 5000.0f;
		DetectionRange = 3000.0f;
		AttackRange = 300.0f;
		MovementSpeed = 600.0f;
		HuntingSpeed = 900.0f;
		AttackDamage = 75.0f;
		RestDuration = 30.0f;
		FeedingDuration = 15.0f;
	}
};

/**
 * T-Rex AI Controller with sophisticated hunting behavior
 * Patrols territory, hunts prey, and exhibits realistic predator behavior
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_TRexBehavior : public AAIController
{
	GENERATED_BODY()

public:
	ANPC_TRexBehavior();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;

	// AI Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components") 
	class UBlackboardComponent* BlackboardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	class UAIPerceptionComponent* AIPerceptionComponent;

	// Behavior Tree Asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
	class UBehaviorTree* TRexBehaviorTree;

	// T-Rex Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Behavior")
	FNPC_TRexStats TRexStats;

	// Current State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex State")
	ENPC_TRexState CurrentState;

	// Territory Center
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Territory")
	FVector TerritoryCenter;

	// Current Target
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Hunting")
	AActor* CurrentTarget;

	// Patrol Points
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Patrol")
	TArray<FVector> PatrolPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Patrol")
	int32 CurrentPatrolIndex;

	// Timers
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Timers")
	float StateTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex Timers")
	float LastAttackTime;

public:
	// Behavior Functions
	UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
	void SetState(ENPC_TRexState NewState);

	UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior") 
	void StartPatrolling();

	UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
	void StartHunting(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
	void StartAttacking();

	UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
	void StartFeeding();

	UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
	void StartResting();

	// Territory Management
	UFUNCTION(BlueprintCallable, Category = "T-Rex Territory")
	void SetTerritoryCenter(FVector NewCenter);

	UFUNCTION(BlueprintCallable, Category = "T-Rex Territory")
	void GeneratePatrolPoints();

	UFUNCTION(BlueprintCallable, Category = "T-Rex Territory")
	bool IsInTerritory(FVector Location) const;

	// Target Detection
	UFUNCTION(BlueprintCallable, Category = "T-Rex Detection")
	AActor* FindNearestPrey();

	UFUNCTION(BlueprintCallable, Category = "T-Rex Detection")
	float GetDistanceToTarget(AActor* Target) const;

	UFUNCTION(BlueprintCallable, Category = "T-Rex Detection")
	bool CanSeeTarget(AActor* Target) const;

	// Combat
	UFUNCTION(BlueprintCallable, Category = "T-Rex Combat")
	void PerformAttack();

	UFUNCTION(BlueprintCallable, Category = "T-Rex Combat")
	bool IsInAttackRange(AActor* Target) const;

	// Movement
	UFUNCTION(BlueprintCallable, Category = "T-Rex Movement")
	void MoveToNextPatrolPoint();

	UFUNCTION(BlueprintCallable, Category = "T-Rex Movement")
	void SetMovementSpeed(float Speed);

protected:
	// AI Perception Callbacks
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	// State Update Functions
	void UpdatePatrolling(float DeltaTime);
	void UpdateHunting(float DeltaTime);
	void UpdateAttacking(float DeltaTime);
	void UpdateFeeding(float DeltaTime);
	void UpdateResting(float DeltaTime);

	// Utility Functions
	FVector GetRandomPatrolPoint() const;
	bool ShouldRest() const;
	bool ShouldFeed() const;
	void SetupAIPerception();
	void SetupBlackboard();
};