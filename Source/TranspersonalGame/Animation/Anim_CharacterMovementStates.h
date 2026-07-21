#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Anim_CharacterMovementStates.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

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

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundDistance;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        bIsAccelerating = false;
        CurrentState = EAnim_MovementState::Idle;
        Velocity = FVector::ZeroVector;
        GroundDistance = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterMovementStates : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterMovementStates();

    // Animation Blueprint Interface
    UFUNCTION(BlueprintCallable, Category = "Animation")
    virtual void NativeInitializeAnimation() override;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State Management
    UFUNCTION(BlueprintCallable, Category = "Movement States")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Movement States")
    EAnim_MovementState GetCurrentMovementState() const;

    UFUNCTION(BlueprintCallable, Category = "Movement States")
    void SetMovementState(EAnim_MovementState NewState);

    // Animation Montage Control
    UFUNCTION(BlueprintCallable, Category = "Animation Montages")
    void PlayIdleMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation Montages")
    void PlayWalkMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation Montages")
    void PlayRunMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation Montages")
    void PlayJumpMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation Montages")
    void StopAllMontages();

    // Blend Space Control
    UFUNCTION(BlueprintCallable, Category = "Blend Spaces")
    void UpdateLocomotionBlendSpace();

protected:
    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    // Movement Data
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UBlendSpace* LocomotionBlendSpace;

    // State Transition Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State Transitions")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State Transitions")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State Transitions")
    float IdleSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State Transitions")
    float JumpVelocityThreshold;

    // Ground Detection
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground Detection")
    float GroundTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground Detection")
    bool bEnableGroundAdaptation;

private:
    // Internal state tracking
    EAnim_MovementState PreviousState;
    float StateChangeTime;
    bool bIsTransitioning;

    // Helper functions
    void UpdateMovementData();
    void PerformGroundTrace();
    bool ShouldTransitionState(EAnim_MovementState NewState);
    void OnStateChanged(EAnim_MovementState OldState, EAnim_MovementState NewState);
};