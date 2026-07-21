// TranspersonalCharacter.h
// Engine Architect #02 — Cycle 020
// ACharacter subclass: WASD movement, sprint, jump, camera boom, survival stats

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TranspersonalCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS(config=Game)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATranspersonalCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ─── Camera ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ─── Enhanced Input ───────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SprintAction;

    // ─── Movement ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting;

    // ─── Survival Stats ───────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxThirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxFear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float ThirstDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaRegenRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaDrainRate;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsAlive;

    // ─── Survival Functions ───────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TakeSurvivalDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Die();

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStaminaPercent() const;

private:
    // Enhanced Input handlers
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    // Legacy input handlers
    void MoveForward(float Value);
    void MoveRight(float Value);

    void StartSprint();
    void StopSprint();

    void UpdateSurvivalStats();

    FTimerHandle SurvivalTimerHandle;
};
