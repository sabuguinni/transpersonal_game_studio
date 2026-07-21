#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_TribalCharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_TribalMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Hunting     UMETA(DisplayName = "Hunting")
};

UENUM(BlueprintType)
enum class EAnim_TribalEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Tired       UMETA(DisplayName = "Tired"),
    Injured     UMETA(DisplayName = "Injured")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalAnimationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_TribalMovementState MovementState = EAnim_TribalMovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional")
    EAnim_TribalEmotionalState EmotionalState = EAnim_TribalEmotionalState::Calm;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation = FRotator::ZeroRotator;

    FAnim_TribalAnimationData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_TribalMovementState::Idle;
        EmotionalState = EAnim_TribalEmotionalState::Calm;
        HealthPercentage = 1.0f;
        StaminaPercentage = 1.0f;
        FearLevel = 0.0f;
        LeftFootIKLocation = FVector::ZeroVector;
        RightFootIKLocation = FVector::ZeroVector;
        LeftFootIKRotation = FRotator::ZeroRotator;
        RightFootIKRotation = FRotator::ZeroRotator;
    }
};

/**
 * Animation Instance for tribal prehistoric characters
 * Handles movement states, emotional expressions, and survival animations
 * Designed for realistic prehistoric human behavior and survival scenarios
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_TribalCharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_TribalCharacterAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

public:
    // Animation data structure
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_TribalAnimationData AnimData;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ACharacter* OwningCharacter;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    UCharacterMovementComponent* MovementComponent;

    // Animation update functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateEmotionalState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalData();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateFootIK();

    // State transition functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_TribalMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetEmotionalState(EAnim_TribalEmotionalState NewState);

    // Animation event handlers
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnStartCrafting();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnStopCrafting();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnStartHunting();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnStopHunting();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnTakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnStaminaChanged(float NewStamina, float MaxStamina);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnFearLevelChanged(float NewFearLevel);

protected:
    // Internal state tracking
    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    FVector LastVelocity;

    UPROPERTY()
    bool bWasInAir;

    // IK settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKInterpSpeed = 15.0f;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float FearThreshold = 0.5f;

    // Helper functions
    FVector PerformFootTrace(FVector FootLocation, FName SocketName);
    EAnim_TribalMovementState CalculateMovementState();
    EAnim_TribalEmotionalState CalculateEmotionalState();
    float CalculateDirection(FVector Velocity, FRotator Rotation);
};