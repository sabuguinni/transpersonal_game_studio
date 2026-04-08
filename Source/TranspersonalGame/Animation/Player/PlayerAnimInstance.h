#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "Animation/AnimNode_MotionMatching.h"
#include "Animation/AnimNode_TwoWayBlend.h"
#include "Animation/AnimNode_Inertialization.h"
#include "BoneControllers/AnimNode_TwoBoneIK.h"
#include "BoneControllers/AnimNode_LookAt.h"
#include "../Core/AnimationSystemManager.h"
#include "PlayerAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FPlayerMovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsFalling = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsJumping = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsSneaking = false;
};

USTRUCT(BlueprintType)
struct FPlayerIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootIKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootIKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float HipOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LookAtTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LookAtAlpha = 0.0f;
};

USTRUCT(BlueprintType)
struct FPlayerEmotionalData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Emotional")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional")
    float StressLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional")
    float ExhaustionLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional")
    float AlertnessLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional")
    bool bIsInDanger = false;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional")
    bool bCanSeeThreats = false;

    UPROPERTY(BlueprintReadOnly, Category = "Emotional")
    float HeartRate = 60.0f; // BPM for breathing animation
};

/**
 * Player Animation Instance for the Jurassic survival game
 * Implements Motion Matching, IK foot placement, and emotional state blending
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaTimeX) override;

public:
    // Animation Data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FPlayerMovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FPlayerIKData IKData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FPlayerEmotionalData EmotionalData;

    // Motion Matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> IdleDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> CrouchDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> FearDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float MotionMatchingBlendTime = 0.2f;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float HipOffsetInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName HipBoneName = TEXT("pelvis");

    // Emotional Animation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Animation")
    float EmotionalBlendSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Animation")
    float BreathingIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Animation")
    float TremorIntensityMultiplier = 1.0f;

    // Look At System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At")
    float LookAtBlendSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At")
    float LookAtMaxAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At")
    FName HeadBoneName = TEXT("head");

protected:
    // Component references
    UPROPERTY()
    TObjectPtr<UAnimationSystemManager> AnimationManager;

    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    // Internal state
    FVector PreviousVelocity;
    float TimeSinceLastMovement;
    bool bWasMovingLastFrame;

    // IK trace results
    FHitResult LeftFootHit;
    FHitResult RightFootHit;
    bool bLeftFootHitValid;
    bool bRightFootHitValid;

    // Emotional state timers
    float FearAccumulationTimer;
    float StressDecayTimer;
    float LastThreatDetectionTime;

private:
    // Update functions
    void UpdateMovementData(float DeltaTime);
    void UpdateIKData(float DeltaTime);
    void UpdateEmotionalData(float DeltaTime);
    void UpdateLookAtData(float DeltaTime);

    // IK helper functions
    void PerformFootIKTrace(const FName& FootBoneName, FHitResult& OutHit, bool& bOutHitValid);
    void CalculateFootIKTransform(const FHitResult& Hit, bool bHitValid, 
                                  FVector& OutLocation, FRotator& OutRotation, float& OutAlpha);
    void CalculateHipOffset();

    // Emotional system helpers
    void DetectThreats();
    void UpdateFearLevel(float DeltaTime);
    void UpdateStressLevel(float DeltaTime);
    void CalculateHeartRate();

    // Motion Matching helpers
    UPoseSearchDatabase* GetCurrentDatabase() const;
    float CalculateMotionMatchingBlendTime() const;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void SetLookAtTarget(const FVector& Target, float BlendTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void ClearLookAtTarget(float BlendTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void TriggerFearResponse(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void AddStress(float Amount);

    UFUNCTION(BlueprintPure, Category = "Player Animation")
    bool IsInFearState() const;

    UFUNCTION(BlueprintPure, Category = "Player Animation")
    float GetCurrentFearLevel() const { return EmotionalData.FearLevel; }

    UFUNCTION(BlueprintPure, Category = "Player Animation")
    float GetCurrentStressLevel() const { return EmotionalData.StressLevel; }
};