// TranspersonalCharacter.h — Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260630_001
// Prehistoric survival game character with integrated SurvivalComponent
// #pragma once

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/Survival/SurvivalComponent.h"
#include "TranspersonalCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USurvivalComponent;

/**
 * ATranspersonalCharacter
 * Main playable character for the prehistoric survival game.
 * Integrates SurvivalComponent for hunger, thirst, stamina, health, fear tracking.
 * Uses standard UE5 ACharacter with WASD movement, camera boom, and follow camera.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATranspersonalCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // ── Camera ──────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ── Survival Component ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // ── Movement State ───────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsExhausted;

    // ── Input Actions ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveForward(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveRight(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void Turn(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void LookUp(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSprinting();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSprinting();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartCrouching();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopCrouching();

    // ── Survival Actions ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AttemptEat();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AttemptDrink();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFearLevel() const;

    // ── Combat / Damage ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeSurvivalDamage(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAlive() const;

protected:
    void UpdateMovementSpeed();
    void HandleStaminaDrain(float DeltaTime);

private:
    float SprintStaminaDrainRate;
    float StaminaRecoveryRate;
};
