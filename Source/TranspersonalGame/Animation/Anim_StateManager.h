#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_StateManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Combat      UMETA(DisplayName = "Combat"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_EmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured"),
    Focused     UMETA(DisplayName = "Focused"),
    Relaxed     UMETA(DisplayName = "Relaxed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_StateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bRequiresGrounded;

    FAnim_StateTransition()
    {
        FromState = EAnim_MovementState::Idle;
        ToState = EAnim_MovementState::Walking;
        BlendTime = 0.2f;
        bRequiresGrounded = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_StateManager : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_StateManager();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    // Current animation states
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_EmotionalState CurrentEmotionalState;

    // Movement parameters
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    // State transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    TArray<FAnim_StateTransition> StateTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float IdleThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunThreshold;

    // Animation blending
    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float IdlePlayRate;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float WalkPlayRate;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float RunPlayRate;

    // State management functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateEmotionalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool CanTransitionTo(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ForceStateTransition(EAnim_MovementState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetStateBlendWeight(EAnim_MovementState State) const;

protected:
    // Internal state tracking
    float StateTimer;
    EAnim_MovementState PreviousMovementState;
    float TransitionTimer;
    bool bIsTransitioning;

    // Helper functions
    void CalculateMovementParameters();
    void UpdateStateTimer(float DeltaTime);
    EAnim_MovementState DetermineMovementState();
    void HandleStateTransition(EAnim_MovementState NewState);
};