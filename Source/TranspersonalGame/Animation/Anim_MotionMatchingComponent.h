#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TurnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float GroundSlope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    ESurvivalActionState ActionState;

    FAnim_MotionMatchingData()
    {
        Velocity = 0.0f;
        Acceleration = 0.0f;
        Direction = FVector::ZeroVector;
        TurnRate = 0.0f;
        bIsGrounded = true;
        GroundSlope = 0.0f;
        ActionState = ESurvivalActionState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PoseSearchData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    TArray<FVector> BonePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    TArray<FVector> BoneVelocities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    TArray<FQuat> BoneRotations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    float PoseScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    int32 AnimationIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    float TimeStamp;

    FAnim_PoseSearchData()
    {
        PoseScore = 0.0f;
        AnimationIndex = -1;
        TimeStamp = 0.0f;
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

    // Motion Matching Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TrajectoryTimeHorizon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float HistoryTimeHorizon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 MaxPoseSearchResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseMatchingThreshold;

    // Current Motion Data
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionMatchingData CurrentMotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    TArray<FAnim_PoseSearchData> PoseSearchHistory;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TArray<class UAnimSequence*> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<ESurvivalActionState, class UAnimMontage*> ActionMontages;

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(float Velocity, FVector Direction, float Acceleration);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_PoseSearchData FindBestPoseMatch(const FAnim_MotionMatchingData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayActionMontage(ESurvivalActionState ActionState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void StopActionMontage();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsActionMontageActive() const;

    // IK and Foot Placement
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector GetFootPlacementLocation(bool bIsLeftFoot) const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetFootIKAlpha(bool bIsLeftFoot) const;

private:
    // Internal Motion Matching
    void AnalyzeCurrentPose();
    float CalculatePoseScore(const FAnim_PoseSearchData& PoseA, const FAnim_PoseSearchData& PoseB) const;
    void UpdateTrajectoryPrediction(float DeltaTime);

    // IK Data
    UPROPERTY()
    float LeftFootIKAlpha;

    UPROPERTY()
    float RightFootIKAlpha;

    UPROPERTY()
    FVector LeftFootIKLocation;

    UPROPERTY()
    FVector RightFootIKLocation;

    // Component References
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Motion Matching State
    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    TArray<FVector> TrajectoryHistory;

    UPROPERTY()
    TArray<FVector> TrajectoryPrediction;
};