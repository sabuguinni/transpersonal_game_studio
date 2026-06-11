#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

// Motion matching data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    ESurvival_MovementState MovementState;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = ESurvival_MovementState::Idle;
    }
};

// Animation state for motion matching
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_StateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    FAnim_MotionData MotionData;

    FAnim_StateData()
    {
        AnimSequence = nullptr;
        PlayRate = 1.0f;
        BlendWeight = 0.0f;
    }
};

/**
 * Motion Matching Animation System
 * Provides fluid character animation based on movement data
 */
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

    // Core motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(const FVector& CurrentVelocity, bool bInAir, bool bCrouching);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_StateData FindBestMatchingAnimation(const FAnim_MotionData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToAnimation(UAnimSequence* TargetAnimation, float BlendTime = 0.2f);

    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetMovementState(ESurvival_MovementState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation State")
    ESurvival_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    // IK and procedural animation
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    FVector GetFootIKOffset(bool bRightFoot) const;

    // Animation database management
    UFUNCTION(BlueprintCallable, Category = "Animation Database")
    void BuildAnimationDatabase();

    UFUNCTION(BlueprintCallable, Category = "Animation Database")
    void AddAnimationToDatabase(UAnimSequence* Animation, const FAnim_MotionData& MotionData);

protected:
    // Current motion data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    // Current animation state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    ESurvival_MovementState CurrentMovementState;

    // Animation database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Database")
    TArray<FAnim_StateData> AnimationDatabase;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CrouchMontage;

    // IK system properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKInterpSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK System")
    FVector LeftFootIKOffset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK System")
    FVector RightFootIKOffset;

    // Motion matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float StateWeight;

    // Internal functions
    float CalculateMotionScore(const FAnim_MotionData& A, const FAnim_MotionData& B) const;
    void PerformFootTrace(bool bRightFoot, FVector& OutIKOffset) const;
    USkeletalMeshComponent* GetOwnerSkeletalMesh() const;
};