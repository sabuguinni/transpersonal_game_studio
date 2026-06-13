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
struct TRANSPERSONALGAME_API FAnim_MotionState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MovementMode MovementMode;

    FAnim_MotionState()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementMode = EAnim_MovementMode::Walking;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendWeights
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float IdleWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float WalkWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float RunWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float JumpWeight;

    FAnim_BlendWeights()
    {
        IdleWeight = 1.0f;
        WalkWeight = 0.0f;
        RunWeight = 0.0f;
        JumpWeight = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionState CurrentMotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Blend Weights")
    FAnim_BlendWeights BlendWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float BlendSpeed;

private:
    void UpdateMotionState(float DeltaTime);
    void UpdateBlendWeights(float DeltaTime);
    float CalculateDirection();
    EAnim_MovementMode DetermineMovementMode();
};