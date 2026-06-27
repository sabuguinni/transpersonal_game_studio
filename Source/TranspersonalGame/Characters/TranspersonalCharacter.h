// TranspersonalCharacter.h
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260627_009
// Playable prehistoric survival character — ACharacter subclass with camera, movement, survival stats.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "TranspersonalCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

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

    // ─── Camera ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ─── Survival Stats ───────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Health")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Health")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Hunger")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Hunger")
    float MaxHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thirst")
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thirst")
    float MaxThirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stamina")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stamina")
    float MaxStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Fear")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Fear")
    float MaxFear;

    // ─── State Flags ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsRunning;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsAlive;

    // ─── Stat Drain Rates ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainRate;

    // ─── Survival Actions ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Die();

    // ─── Movement Actions ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartRunning();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopRunning();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ToggleCrouch();

private:
    void MoveForward(float Value);
    void MoveRight(float Value);
    void TickSurvivalStats();

    FTimerHandle SurvivalTimerHandle;
};
