// TranspersonalCharacter.h
// Transpersonal Game Studio — Core Systems Programmer #03
// Prehistoric survival game player character with integrated SurvivalComponent

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TranspersonalCharacter.generated.h"

// Forward declarations
class UCameraComponent;
class USpringArmComponent;
class USurvivalComponent;

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

    // ─── Camera ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ─── Survival Component ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // ─── Survival Stats (exposed for HUD) ─────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float MaxHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float MaxThirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float MaxStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float MaxFear;

    // ─── Movement ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsSprinting;

    // ─── Input Handlers ───────────────────────────────────────────────────────
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
    void StartJump();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopJump();

    // ─── Survival Actions ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeWater(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStaminaPercent() const;

private:
    // Internal survival state (managed by SurvivalComp)
    float CurrentHealth;
    float CurrentHunger;
    float CurrentThirst;
    float CurrentStamina;
    float CurrentFear;

    void HandleDeath();
    void UpdateMovementSpeed();
};
