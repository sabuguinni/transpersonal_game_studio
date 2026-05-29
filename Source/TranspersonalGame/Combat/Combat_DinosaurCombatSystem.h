#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Combat_DinosaurCombatSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Hunting UMETA(DisplayName = "Hunting"),
	Attacking UMETA(DisplayName = "Attacking"),
	Fleeing UMETA(DisplayName = "Fleeing"),
	Feeding UMETA(DisplayName = "Feeding")
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurCombatStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
	float AttackDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
	float DetectionRadius = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
	float AttackRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
	float MovementSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
	float AggressionLevel = 0.7f;

	FCombat_DinosaurCombatStats()
	{
		Health = 100.0f;
		MaxHealth = 100.0f;
		AttackDamage = 25.0f;
		DetectionRadius = 5000.0f;
		AttackRange = 300.0f;
		MovementSpeed = 600.0f;
		AggressionLevel = 0.7f;
	}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatSystem : public AActor
{
	GENERATED_BODY()

public:
	ACombat_DinosaurCombatSystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Components")
	USphereComponent* DetectionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
	FCombat_DinosaurCombatStats CombatStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
	ECombat_DinosaurCombatState CurrentCombatState;

	UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
	AActor* CurrentTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
	bool bIsAggressive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
	bool bCanFlee = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
	float FleeHealthThreshold = 20.0f;

	UFUNCTION(BlueprintCallable, Category = "Combat Actions")
	void StartHunting(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Combat Actions")
	void AttackTarget();

	UFUNCTION(BlueprintCallable, Category = "Combat Actions")
	void FleeFromThreat();

	UFUNCTION(BlueprintCallable, Category = "Combat Actions")
	void TakeDamage(float DamageAmount, AActor* DamageSource);

	UFUNCTION(BlueprintCallable, Category = "Combat Detection")
	AActor* DetectNearbyPlayer();

	UFUNCTION(BlueprintCallable, Category = "Combat Detection")
	bool IsTargetInAttackRange();

	UFUNCTION(BlueprintCallable, Category = "Combat State")
	void SetCombatState(ECombat_DinosaurCombatState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
	void OnCombatStateChanged(ECombat_DinosaurCombatState OldState, ECombat_DinosaurCombatState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
	void OnTargetAcquired(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
	void OnAttackExecuted(AActor* Target);

private:
	FTimerHandle AttackCooldownTimer;
	float LastAttackTime = 0.0f;
	float AttackCooldown = 2.0f;

	void UpdateCombatBehavior();
	void ProcessHuntingState();
	void ProcessAttackingState();
	void ProcessFleeingState();
	bool CanAttack() const;
};