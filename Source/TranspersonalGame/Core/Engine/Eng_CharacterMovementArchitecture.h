#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "Eng_CharacterMovementArchitecture.generated.h"

/**
 * Engine Architect Character Movement Architecture
 * Defines the core movement system for TranspersonalCharacter
 * Implements WASD movement, camera controls, and basic physics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_CharacterMovementArchitecture : public ACharacter
{
    GENERATED_BODY()

public:
    AEng_CharacterMovementArchitecture();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    // CAMERA SYSTEM
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

    // MOVEMENT PARAMETERS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BaseTurnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BaseLookUpRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpVelocity;

    // SURVIVAL STATS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst;

    // INPUT ACTIONS
    UFUNCTION()
    void MoveForward(float Value);

    UFUNCTION()
    void MoveRight(float Value);

    UFUNCTION()
    void TurnAtRate(float Rate);

    UFUNCTION()
    void LookUpAtRate(float Rate);

    UFUNCTION()
    void StartJump();

    UFUNCTION()
    void StopJump();

    UFUNCTION()
    void StartRun();

    UFUNCTION()
    void StopRun();

    // SURVIVAL FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreHealth(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeStamina(float StaminaCost);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float StaminaAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateHunger(float HungerChange);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateThirst(float ThirstChange);

    // GETTERS
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHealthPercentage() const { return Health / MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStaminaPercentage() const { return Stamina / MaxStamina; }

    UFUNCTION(BlueprintPure, Category = "Camera")
    class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    UFUNCTION(BlueprintPure, Category = "Camera")
    class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
    // INTERNAL STATE
    bool bIsRunning;
    float StaminaRegenRate;
    float HungerDecayRate;
    float ThirstDecayRate;

    // MOVEMENT VALIDATION
    UFUNCTION()
    void ValidateMovementComponent();

    UFUNCTION()
    void InitializeDefaultValues();
};