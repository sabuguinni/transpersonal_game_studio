#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TranspersonalCharacter.generated.h"

USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Thirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Stamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Fear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Temperature = 37.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATranspersonalCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	// ─── Camera ───────────────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
		meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
		meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// ─── Survival Component ───────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival",
		meta = (AllowPrivateAccess = "true"))
	class USurvivalComponent* SurvivalComp;

	// ─── Survival Stats (direct access for Blueprint) ─────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	FCore_SurvivalStats SurvivalStats;

	// ─── Movement flags ───────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting;

	// ─── Input handlers ───────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveForward(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveRight(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void TurnAtRate(float Rate);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void LookUpAtRate(float Rate);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	// ─── Survival actions ─────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void ApplyDamageToSurvival(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetHealthPercent() const;

	// ─── Camera settings ──────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float BaseTurnRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float BaseLookUpRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 600.0f;

private:
	void TickSurvivalDecay(float DeltaTime);
};
