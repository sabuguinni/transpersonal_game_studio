#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/Survival/SurvivalComponent.h"
#include "TranspersonalCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

/**
 * ATranspersonalCharacter
 * Player-controlled prehistoric human survivor.
 * Extends ACharacter with survival stats (health, hunger, thirst, stamina, fear)
 * via USurvivalComponent. Uses UE5 Enhanced Input for WASD + jump + run.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATranspersonalCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Survival Component ─────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival",
              meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // ── Camera ─────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
              meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
              meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ── Movement Speeds ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed = 150.0f;

    // ── Input ──────────────────────────────────────────────────────────────
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* CrouchAction;

    // ── State ──────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting = false;

    // ── Blueprint-callable helpers ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHunger() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirst() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStamina() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSprint();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSprint();

private:
    void Move(const struct FInputActionValue& Value);
    void Look(const struct FInputActionValue& Value);
    void HandleSprintStart();
    void HandleSprintStop();
};
