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
 * Player-controlled primitive human character for the prehistoric survival game.
 * Supports WASD movement, jump, run, and survival stats via SurvivalComponent.
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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
              meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
              meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ── Survival Component ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival",
              meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // ── Movement ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsRunning;

    // ── Survival Stats (mirrored from SurvivalComponent for Blueprint access) ──
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Health;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Hunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Thirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Stamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Fear;

    // ── Input Handlers ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Input")
    void MoveForward(float Value);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void MoveRight(float Value);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void LookUp(float Value);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void LookRight(float Value);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void StartRun();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void StopRun();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void StartJump();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void StopJump();

    // ── Damage ───────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage_Survival(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAlive() const;

private:
    void UpdateSurvivalStats(float DeltaTime);
    void UpdateMovementSpeed();
};
