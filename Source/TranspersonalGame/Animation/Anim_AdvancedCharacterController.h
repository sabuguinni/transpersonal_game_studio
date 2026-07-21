#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/PoseAsset.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_AdvancedCharacterController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Dodging         UMETA(DisplayName = "Dodging"),
    Interacting     UMETA(DisplayName = "Interacting"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering")
};

USTRUCT(BlueprintType)
struct FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float KneeDirection;

    FAnim_IKFootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        KneeDirection = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_MotionMatchingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector CurrentVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector DesiredVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector FacingDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FVector> TrajectoryPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MotionMatchingWeight;

    FAnim_MotionMatchingData()
    {
        CurrentVelocity = FVector::ZeroVector;
        DesiredVelocity = FVector::ZeroVector;
        FacingDirection = FVector::ForwardVector;
        MotionMatchingWeight = 1.0f;
    }
};

/**
 * Advanced Character Animation Controller
 * Handles complex animation states, IK systems, and Motion Matching
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_AdvancedCharacterController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_AdvancedCharacterController();

protected:
    // === CORE ANIMATION PROPERTIES ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    EAnim_MovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Action", meta = (AllowPrivateAccess = "true"))
    EAnim_ActionState ActionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Speed", meta = (AllowPrivateAccess = "true"))
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Speed", meta = (AllowPrivateAccess = "true"))
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsAccelerating;

    // === IK SYSTEM PROPERTIES ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
    FAnim_IKFootData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
    FAnim_IKFootData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
    float HipOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
    bool bEnableFootIK;

    // === MOTION MATCHING PROPERTIES ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Motion Matching", meta = (AllowPrivateAccess = "true"))
    FAnim_MotionMatchingData MotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Motion Matching", meta = (AllowPrivateAccess = "true"))
    bool bUseMotionMatching;

    // === ANIMATION ASSETS ===
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Assets", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* IdleMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Assets", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* WalkMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Assets", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* RunMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Assets", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* JumpMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Assets", meta = (AllowPrivateAccess = "true"))
    UBlendSpace1D* LocomotionBlendSpace;

    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float WalkThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float RunThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float IKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float IKInterpSpeed;

    // === CACHED REFERENCES ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation|References", meta = (AllowPrivateAccess = "true"))
    ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|References", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* CharacterMovement;

public:
    // === CORE ANIMATION FUNCTIONS ===
    
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    UFUNCTION(BlueprintCallable, Category = "Animation|State")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation|State")
    void UpdateActionState(EAnim_ActionState NewActionState);

    // === IK SYSTEM FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void CalculateFootIK(FAnim_IKFootData& FootData, FName SocketName, FName BoneName);

    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void SetEnableFootIK(bool bEnable);

    // === MOTION MATCHING FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Animation|Motion Matching")
    void UpdateMotionMatching(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation|Motion Matching")
    void CalculateTrajectory();

    UFUNCTION(BlueprintCallable, Category = "Animation|Motion Matching")
    void SetUseMotionMatching(bool bUse);

    // === ANIMATION CONTROL FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void PlayActionMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void StopActionMontage(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    bool IsPlayingActionMontage() const;

    // === UTILITY FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation|Utility")
    float GetMovementDirection() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation|Utility")
    bool ShouldMoveForward() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation|Utility")
    FVector GetVelocityDirection() const;

private:
    // === INTERNAL HELPER FUNCTIONS ===
    
    void UpdateBasicMovementValues();
    void UpdateAdvancedMovementValues();
    FVector PerformFootTrace(FName SocketName, float TraceDistance);
    void InterpolateIKValues(FAnim_IKFootData& FootData, const FVector& TargetLocation, const FRotator& TargetRotation, float DeltaTime);
    void CalculateHipOffset();
    void UpdateTrajectoryPrediction();
    float CalculateMotionMatchingScore(const FAnim_MotionMatchingData& TargetData);
};