#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
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
    float TurnRate;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        TurnRate = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* CrouchBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* JumpStartAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* JumpLoopAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* JumpEndAnimation;

    FAnim_BlendSpaceData()
    {
        LocomotionBlendSpace = nullptr;
        CrouchBlendSpace = nullptr;
        IdleAnimation = nullptr;
        JumpStartAnimation = nullptr;
        JumpLoopAnimation = nullptr;
        JumpEndAnimation = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion data calculation
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData CalculateMotionData();

    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateAnimationState(const FAnim_MotionData& MotionData);

    // Blend space management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetupBlendSpaces();

    // Jump animation handling
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void TriggerJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void EndJumpAnimation();

    // IK foot placement
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateFootIK();

    // Animation montage playback
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayAnimationMontage(class UAnimMontage* Montage, float PlayRate = 1.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_BlendSpaceData BlendSpaceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MotionSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DirectionSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float FootIKInterpSpeed;

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Current motion state
    FAnim_MotionData CurrentMotionData;
    FAnim_MotionData PreviousMotionData;

    // IK data
    float LeftFootIKOffset;
    float RightFootIKOffset;
    float PelvisIKOffset;

    // Internal methods
    void CacheComponents();
    void SmoothMotionData(FAnim_MotionData& MotionData, float DeltaTime);
    float CalculateFootIKOffset(const FName& SocketName);
    void ApplyFootIK(float DeltaTime);
};