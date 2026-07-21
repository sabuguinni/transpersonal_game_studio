#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SharedTypes.h"
#include "PrimitiveHunterCharacter.generated.h"

// ============================================================
// Survival stats struct for the primitive hunter character
// ============================================================
USTRUCT(BlueprintType)
struct FChar_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Temperature = 37.0f;
};

// ============================================================
// Movement state enum
// ============================================================
UENUM(BlueprintType)
enum class EChar_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Dead        UMETA(DisplayName = "Dead")
};

// ============================================================
// APrimitiveHunterCharacter — playable prehistoric human
// Third-person character with survival stats, WASD movement,
// sprint, crouch, camera boom, and day/night phase reactions
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrimitiveHunterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APrimitiveHunterCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Camera Components ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // ---- Survival Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FChar_SurvivalStats SurvivalStats;

    // ---- Movement State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    EChar_MovementState MovementState;

    // ---- Movement Settings ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaDrainRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaRegenRate = 5.0f;

    // ---- Survival Drain Rates ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerDrainRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float ThirstDrainRate = 0.8f;

    // ---- Camera Settings ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraBoomLength = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraLagSpeed = 10.0f;

    // ---- State Flags ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsExhausted = false;

    // ---- Input Handlers ----
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveForward(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveRight(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void LookUp(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void Turn(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSprint();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSprint();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ToggleCrouch();

    // ---- Survival Functions ----
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage_Hunter(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStaminaPercent() const;

    // ---- Day Phase Reaction ----
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void OnDayPhaseChanged(int32 NewPhaseIndex);

private:
    void UpdateSurvivalStats(float DeltaTime);
    void UpdateMovementState();
    void UpdateStamina(float DeltaTime);
    bool bCrouchToggle = false;
};
