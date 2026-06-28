// TranspersonalCharacter.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Prehistoric survival character with full survival stat integration

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
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // === SURVIVAL COMPONENT ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    USurvivalComponent* SurvivalComp;

    // === CAMERA ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

    // === MOVEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaDrainPerSecond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaRegenPerSecond;

    bool bIsRunning;

    // === SURVIVAL STATS (replicated from SurvivalComp for Blueprint access) ===
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CurrentHunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CurrentThirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CurrentStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CurrentFear;

    // === BIOME MODIFIERS ===
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BiomeMovementModifier;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BiomeHungerDrainModifier;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BiomeThirstDrainModifier;

    // === INPUT HANDLERS ===
    void MoveForward(float Value);
    void MoveRight(float Value);
    void TurnAtRate(float Rate);
    void LookUpAtRate(float Rate);
    void StartRunning();
    void StopRunning();
    void Jump();
    void StopJumping();

    // === SURVIVAL ACTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeWater(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TakeDamage_Survival(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const;

    // === BIOME INTEGRATION ===
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SetBiomeModifiers(float MovementMod, float HungerMod, float ThirstMod);

    // === SURVIVAL STAT TICK ===
    void TickSurvivalStats(float DeltaTime);
    void UpdateMovementSpeedFromStats();

    // === CAMERA SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float BaseTurnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float BaseLookUpRate;
};
