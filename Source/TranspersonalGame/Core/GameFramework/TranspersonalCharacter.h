// TranspersonalCharacter.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Prehistoric survival game — player character with survival stats and SurvivalComponent integration
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/Survival/SurvivalComponent.h"
#include "TranspersonalCharacter.generated.h"

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

    // ── Survival Component ────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival",
              meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // ── Camera ────────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
              meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
              meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

    // ── Movement parameters ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpZVelocity;

    // ── Survival stats (replicated via SurvivalComp) ─────────────────────────
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

    // ── Input handlers ────────────────────────────────────────────────────────
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
    void StartCrouch();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void StopCrouch();

    // ── Survival actions ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SyncSurvivalStats();

private:
    bool bIsRunning;
    bool bIsCrouching;

    // Drain rates per second
    static constexpr float HungerDrainRate  = 0.5f;
    static constexpr float ThirstDrainRate  = 0.8f;
    static constexpr float StaminaDrainRun  = 5.0f;
    static constexpr float StaminaRegenIdle = 3.0f;

    void TickSurvivalStats(float DeltaTime);
};
