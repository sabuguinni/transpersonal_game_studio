#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AI/NPCBehavior/TRexBehaviorComponent.h"
#include "TRexPawn.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRexPawn : public APawn
{
	GENERATED_BODY()

public:
	ATRexPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Mesh")
	UStaticMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Mesh")
	UStaticMeshComponent* HeadMesh;

	// Behavior component from Agent #11
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|AI")
	UTRexBehaviorComponent* BehaviorComponent;

	// Combat stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Stats")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float MoveSpeed;

	// Damage reception
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetHealthPercent() const;

	// Death
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void OnDeath();
	virtual void OnDeath_Implementation();

	// Roar feedback (called by BehaviorComponent on state change)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TriggerRoar();

protected:
	bool bIsDead;
	float RoarCooldown;
	float TimeSinceLastRoar;
};
