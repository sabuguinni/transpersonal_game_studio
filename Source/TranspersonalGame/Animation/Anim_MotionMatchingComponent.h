#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "Anim_MotionMatchingComponent.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Combat      UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion matching data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FAnim_MotionData PreviousMotionData;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimSequence* JumpStartAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimSequence* JumpLoopAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimSequence* JumpEndAnimation;

    // Motion matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float WalkThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float RunThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MotionSmoothingSpeed = 10.0f;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(float Speed, float Direction, bool bInAir, bool bCrouching);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MovementState GetCurrentMovementState() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData GetMotionData() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMovementState(EAnim_MovementState NewState);

private:
    void CalculateMovementState();
    void SmoothMotionData(float DeltaTime);
};