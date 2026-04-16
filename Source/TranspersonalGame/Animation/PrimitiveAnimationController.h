#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "PrimitiveAnimationController.generated.h"

// Animation states for primitive character
UENUM(BlueprintType)
enum class EAnim_PrimitiveState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Injured     UMETA(DisplayName = "Injured"),
    Dead        UMETA(DisplayName = "Dead")
};

// Animation blend data for smooth transitions
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsBlending;

    FAnim_BlendData()
    {
        BlendTime = 0.3f;
        BlendWeight = 0.0f;
        bIsBlending = false;
    }
};

/**
 * Primitive Animation Controller
 * Manages basic character animations for prehistoric human survival gameplay
 * Handles state transitions, movement blending, and survival-specific animations
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPrimitiveAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPrimitiveAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAnimationState(EAnim_PrimitiveState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_PrimitiveState GetCurrentAnimationState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementAnimation(float Speed, bool bIsInAir, bool bIsCrouching);

    // Survival-specific animations
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayGatheringAnimation(float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayCraftingAnimation(float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAttackAnimation(bool bIsHeavyAttack = false);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayInjuredAnimation(float Severity = 0.5f);

    // Animation blending
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void BlendToState(EAnim_PrimitiveState TargetState, float BlendDuration = 0.3f);

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsBlending() const { return BlendData.bIsBlending; }

    // Movement parameters
    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMovementSpeed() const { return MovementSpeed; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMovementDirection() const { return MovementDirection; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsMoving() const { return MovementSpeed > 5.0f; }

protected:
    // Current animation state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_PrimitiveState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_PrimitiveState PreviousState;

    // Movement data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float MovementSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float MovementDirection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    // Blend data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation Blending")
    FAnim_BlendData BlendData;

    // Animation timing
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation Timing")
    float StateTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation Timing")
    float ActionTimer;

    // Speed thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold;

    // References
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

private:
    // Internal state management
    void UpdateStateTimer(float DeltaTime);
    void UpdateBlending(float DeltaTime);
    void CheckStateTransitions();
    void HandleStateEntry(EAnim_PrimitiveState NewState);
    void HandleStateExit(EAnim_PrimitiveState OldState);

    // Movement analysis
    void AnalyzeMovement();
    float CalculateMovementDirection();
    bool ShouldTransitionToMovementState();

    // Animation utilities
    void ResetActionTimer();
    bool IsActionComplete() const;
    void LogStateChange(EAnim_PrimitiveState From, EAnim_PrimitiveState To);
};