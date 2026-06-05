#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_MotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Combat      UMETA(DisplayName = "Combat"),
    Interaction UMETA(DisplayName = "Interaction")
};

UENUM(BlueprintType)
enum class EAnim_MovementDirection : uint8
{
    Forward     UMETA(DisplayName = "Forward"),
    Backward    UMETA(DisplayName = "Backward"),
    Left        UMETA(DisplayName = "Left"),
    Right       UMETA(DisplayName = "Right"),
    ForwardLeft UMETA(DisplayName = "Forward Left"),
    ForwardRight UMETA(DisplayName = "Forward Right"),
    BackwardLeft UMETA(DisplayName = "Backward Left"),
    BackwardRight UMETA(DisplayName = "Backward Right")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    EAnim_MotionState CurrentState = EAnim_MotionState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    EAnim_MovementDirection MovementDirection = EAnim_MovementDirection::Forward;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_MotionState::Idle;
        MovementDirection = EAnim_MovementDirection::Forward;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace* LocomotionBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace* CrouchedBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace* CombatBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float BlendSpaceX = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float BlendSpaceY = 0.0f;

    FAnim_BlendSpaceData()
    {
        LocomotionBlendSpace = nullptr;
        CrouchedBlendSpace = nullptr;
        CombatBlendSpace = nullptr;
        BlendSpaceX = 0.0f;
        BlendSpaceY = 0.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Core Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(const FVector& InVelocity, const FVector& InAcceleration);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionState(EAnim_MotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MotionState GetCurrentMotionState() const { return CurrentMotionData.CurrentState; }

    // Blend Space Functions
    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void UpdateBlendSpaceValues(float Speed, float Direction);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    FAnim_BlendSpaceData GetBlendSpaceData() const { return BlendSpaceData; }

    // Animation Montage Functions
    UFUNCTION(BlueprintCallable, Category = "Animation Montage")
    void PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation Montage")
    void StopAnimationMontage(UAnimMontage* Montage);

    // Motion State Analysis
    UFUNCTION(BlueprintCallable, Category = "Motion Analysis")
    EAnim_MovementDirection CalculateMovementDirection(const FVector& Velocity, const FVector& ForwardVector);

    UFUNCTION(BlueprintCallable, Category = "Motion Analysis")
    bool IsMovementSignificant(float MinSpeed = 10.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Motion Analysis")
    float CalculateMovementAngle(const FVector& Velocity, const FVector& ForwardVector);

protected:
    // Motion Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data", meta = (AllowPrivateAccess = "true"))
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data", meta = (AllowPrivateAccess = "true"))
    FAnim_MotionData PreviousMotionData;

    // Blend Space Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blend Space", meta = (AllowPrivateAccess = "true"))
    FAnim_BlendSpaceData BlendSpaceData;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<EAnim_MotionState, UAnimSequence*> StateAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<FString, UAnimMontage*> AnimationMontages;

    // Motion Matching Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float MotionMatchingUpdateRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float BlendSpaceSmoothingSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float MinimumMovementSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float SprintSpeedThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float RunSpeedThreshold = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float WalkSpeedThreshold = 50.0f;

private:
    // Internal state tracking
    float LastUpdateTime = 0.0f;
    FVector LastVelocity = FVector::ZeroVector;
    FVector LastAcceleration = FVector::ZeroVector;

    // Helper functions
    void AnalyzeMotionState();
    void UpdateBlendSpaceSmoothing(float DeltaTime);
    EAnim_MotionState DetermineMotionState(float Speed, bool bIsInAir, bool bIsCrouching);
};