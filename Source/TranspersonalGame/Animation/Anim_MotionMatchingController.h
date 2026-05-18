#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingController.generated.h"

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
    Crawling    UMETA(DisplayName = "Crawling")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsOnGround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_MovementState CurrentState;

    FAnim_MotionMatchingData()
    {
        Velocity = 0.0f;
        Direction = 0.0f;
        Acceleration = 0.0f;
        bIsOnGround = true;
        bIsCrouching = false;
        CurrentState = EAnim_MovementState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector LeftFootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector RightFootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator LeftFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator RightFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float LeftFootAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float RightFootAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float HipOffset;

    FAnim_IKFootData()
    {
        LeftFootLocation = FVector::ZeroVector;
        RightFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootAlpha = 0.0f;
        RightFootAlpha = 0.0f;
        HipOffset = 0.0f;
    }
};

/**
 * Motion Matching Controller for fluid character animation
 * Handles advanced animation blending, IK foot placement, and state transitions
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion Matching Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionMatchingData MotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float StateChangeThreshold;

    // IK Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FAnim_IKFootData FootIKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> LocomotionBlendSpace;

    // Component References
    UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USkeletalMeshComponent> MeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

public:
    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void TransitionToState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MovementState GetCurrentMovementState() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetMovementDirection() const;

    // IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "IK")
    void PerformFootTrace(const FVector& FootLocation, const FName& SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetFootIKEnabled(bool bEnabled);

    // Animation Control Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAnimationMontage(UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsPlayingMontage(UAnimMontage* Montage) const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void InitializeComponents();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ResetAnimationState();

private:
    // Internal state tracking
    EAnim_MovementState PreviousState;
    float StateTransitionTimer;
    bool bIsInitialized;

    // IK calculation helpers
    void CalculateFootIKOffset(const FVector& FootLocation, const FName& SocketName, FVector& OutOffset, FRotator& OutRotation);
    float CalculateHipOffset();
    
    // Motion matching helpers
    EAnim_MovementState DetermineMovementState();
    void SmoothStateTransition(float DeltaTime);
};