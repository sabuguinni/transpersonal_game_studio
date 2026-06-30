// TranspersonalCharacter.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Player character: WASD movement, sprint, survival stats (health/hunger/thirst/stamina/fear)
// Inherits from ACharacter — uses UCharacterMovementComponent, SpringArm, Camera

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TranspersonalCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS(config=Game, BlueprintType, Blueprintable)
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

    // ─── Input Mapping ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SprintAction;

    // ─── Survival Stats ───────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHunger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxThirst;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxFear;

    // ─── Depletion Rates ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDepletionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDepletionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDepletionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float SurvivalTickInterval;

    // ─── State ────────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    bool bIsSprinting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    bool bIsAlive;

    // ─── Public Methods ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float ApplyDamage(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void HealCharacter(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeWater(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSprint();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSprint();

    // ─── Blueprint Events ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "Survival")
    void OnSurvivalStatsUpdated();
    virtual void OnSurvivalStatsUpdated_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Survival")
    void OnHealthChanged(float NewHealth, float NewMaxHealth);
    virtual void OnHealthChanged_Implementation(float NewHealth, float NewMaxHealth);

    UFUNCTION(BlueprintNativeEvent, Category = "Survival")
    void OnCharacterDied();
    virtual void OnCharacterDied_Implementation();

private:
    FTimerHandle SurvivalTickHandle;

    void TickSurvivalStats();
    void UpdateStamina(float DeltaTime);
    void Die();

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
};
