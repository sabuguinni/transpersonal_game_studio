#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Shared/SharedTypes.h"
#include "Anim_MotionMatchingController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MovementState MovementState;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TrajectoryWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 SearchFrameCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    FAnim_MotionMatchingSettings()
    {
        VelocityWeight = 1.0f;
        DirectionWeight = 0.8f;
        PoseWeight = 0.6f;
        TrajectoryWeight = 1.2f;
        SearchFrameCount = 30;
        BlendTime = 0.2f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData PreviousMotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionMatchingSettings MotionMatchingSettings;

    // Movement State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsWalking;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsRunning;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsJumping;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsFalling;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsLanding;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsClimbing;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSwimming;

    // Speed Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Thresholds")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Thresholds")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Thresholds")
    float SprintSpeedThreshold;

    // Animation Blend Values
    UPROPERTY(BlueprintReadOnly, Category = "Animation Blends")
    float ForwardBackwardBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blends")
    float LeftRightBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blends")
    float MovementBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blends")
    float SpeedBlend;

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void CalculateMovementBlends();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMovementStates();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData GetBestMotionMatch(const FAnim_MotionData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionData& Motion1, const FAnim_MotionData& Motion2);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Animation Utils")
    float GetMovementDirection() const;

    UFUNCTION(BlueprintPure, Category = "Animation Utils")
    bool ShouldTransitionToState(EAnim_MovementState NewState) const;

    UFUNCTION(BlueprintCallable, Category = "Animation Utils")
    void SetMovementState(EAnim_MovementState NewState);

private:
    void InitializeSpeedThresholds();
    void UpdateCharacterReferences();
    float CalculateDirectionAngle() const;
    void SmoothBlendValues(float DeltaTime);

    // Internal state tracking
    float StateTransitionTimer;
    float BlendSmoothingSpeed;
    bool bWasMovingLastFrame;
    FVector LastVelocity;
};