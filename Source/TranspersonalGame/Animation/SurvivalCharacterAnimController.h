#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SurvivalCharacterAnimController.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Panicked    UMETA(DisplayName = "Panicked"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured")
};

USTRUCT(BlueprintType)
struct FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsFalling = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsJumping = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsFalling = false;
        bIsJumping = false;
        bIsCrouching = false;
    }
};

USTRUCT(BlueprintType)
struct FAnim_SurvivalData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaLevel = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthLevel = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured = false;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted = false;

    FAnim_SurvivalData()
    {
        FearLevel = 0.0f;
        StaminaLevel = 1.0f;
        HealthLevel = 1.0f;
        bIsInjured = false;
        bIsExhausted = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalCharacterAnimController : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalCharacterAnimController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main update function
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationData();

    // State management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_SurvivalMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetEmotionalState(EAnim_SurvivalEmotionalState NewState);

    // Animation triggers
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerLandAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerFearReaction();

    // Getters for animation blueprint
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_SurvivalData GetSurvivalData() const { return SurvivalData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_SurvivalMovementState GetMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_SurvivalEmotionalState GetEmotionalState() const { return CurrentEmotionalState; }

protected:
    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    // Animation data
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_SurvivalData SurvivalData;

    // Current states
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    EAnim_SurvivalMovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    EAnim_SurvivalEmotionalState CurrentEmotionalState;

    // Animation montages
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* FearReactionMontage;

    // Blend spaces
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class UBlendSpace* CrouchBlendSpace;

private:
    // Internal update functions
    void UpdateMovementData();
    void UpdateSurvivalData();
    void UpdateMovementState();
    void UpdateEmotionalState();

    // Transition timers
    float StateTransitionTimer;
    float EmotionalTransitionTimer;
};