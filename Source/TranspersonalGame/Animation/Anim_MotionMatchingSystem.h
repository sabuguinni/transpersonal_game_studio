#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

// Animation state tracking for motion matching
UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Climbing    UMETA(DisplayName = "Climbing")
};

// Animation blend parameters for smooth transitions
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float LeanAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TimeSinceLastFootstep = 0.0f;

    FAnim_BlendParameters()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        LeanAngle = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        TimeSinceLastFootstep = 0.0f;
    }
};

// Motion matching data for animation selection
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FRotator RotationVelocity = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_MovementState CurrentState = EAnim_MovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_MovementState PreviousState = EAnim_MovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float StateTransitionTime = 0.0f;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        RotationVelocity = FRotator::ZeroRotator;
        CurrentState = EAnim_MovementState::Idle;
        PreviousState = EAnim_MovementState::Idle;
        StateTransitionTime = 0.0f;
    }
};

/**
 * Motion Matching Animation System
 * Provides fluid, realistic character animation using motion matching principles
 * Analyzes character movement and selects appropriate animations for seamless blending
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    // Animation Blueprint Interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Motion matching core data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData MotionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_BlendParameters BlendParams;

    // Character references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    class ACharacter* OwningCharacter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    // Animation assets
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> LocomotionBlendSpace;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> IdleMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> JumpMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> LandMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> GatherMontage;

    // Motion matching settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float SpeedThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float RunThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float DirectionSmoothingSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float BlendSpeed = 5.0f;

public:
    // Motion matching interface
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MovementState CalculateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateBlendParameters(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool ShouldTransitionToState(EAnim_MovementState NewState);

    // Animation playback
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActionMontage(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsPlayingActionMontage() const;

    // State queries
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsMoving() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsRunning() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsInAir() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsCrouching() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetMovementDirection() const;

private:
    // Internal state tracking
    FVector LastVelocity;
    FVector LastAcceleration;
    float StateTimer;
    float LastGroundTime;
    bool bWasInAir;

    // Helper functions
    float CalculateDirection(const FVector& Velocity, const FRotator& Rotation);
    void SmoothBlendParameters(float DeltaTime);
    bool ValidateStateTransition(EAnim_MovementState FromState, EAnim_MovementState ToState);
};