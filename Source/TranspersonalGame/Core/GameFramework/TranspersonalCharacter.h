#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TranspersonalCharacter.generated.h"

// Forward declarations
class UCameraComponent;
class USpringArmComponent;
class USurvivalComponent;

/**
 * ATranspersonalCharacter
 * Player character for the prehistoric survival game.
 * Supports WASD movement, camera boom, jump, sprint,
 * and wires into the SurvivalComponent for hunger/thirst/health/stamina/fear.
 */
UCLASS(BlueprintType, Blueprintable)
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

    // ── Camera ──────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ── Survival ─────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // ── Movement settings ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed;

    // ── State ────────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsExhausted;

    // ── Input handlers ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveForward(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveRight(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void Turn(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void LookUp(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSprint();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSprint();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartCrouch();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopCrouch();

    // ── Survival queries (Blueprint-accessible) ───────────────────────────────
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    float GetHealth() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    float GetHunger() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    float GetThirst() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    float GetStamina() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    float GetFear() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const;

private:
    // Stamina drain/recovery per second
    static constexpr float StaminaDrainRate  = 15.0f;
    static constexpr float StaminaRegenRate  = 8.0f;
    static constexpr float ExhaustionThreshold = 10.0f;

    void UpdateSprintState(float DeltaTime);
};
