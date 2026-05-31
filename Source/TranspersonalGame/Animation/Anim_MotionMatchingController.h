#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Anim_MotionMatchingController.generated.h"

/**
 * Motion data structure for matching system
 * Stores pose and trajectory information for realistic movement
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    // Current pose bone positions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    TArray<FVector> BonePositions;

    // Current pose bone velocities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    TArray<FVector> BoneVelocities;

    // Future trajectory points
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    TArray<FVector> TrajectoryPoints;

    // Future trajectory velocities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    TArray<FVector> TrajectoryVelocities;

    // Animation sequence reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    UAnimSequence* AnimSequence;

    // Time in animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float AnimTime;

    // Quality score for this motion
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float MatchScore;

    FAnim_MotionData()
    {
        AnimSequence = nullptr;
        AnimTime = 0.0f;
        MatchScore = 0.0f;
    }
};

/**
 * Motion Matching Controller for realistic character animation
 * Implements simplified motion matching for fluid character movement
 * Based on research from "Motion Matching for Real-Time Character Animation"
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UObject
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

protected:
    // Motion database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionData> MotionDatabase;

    // Animation sequences for different movement types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    TArray<UAnimSequence*> IdleAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    TArray<UAnimSequence*> WalkAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    TArray<UAnimSequence*> RunAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    TArray<UAnimSequence*> JumpAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sequences")
    TArray<UAnimSequence*> TurnAnimations;

    // Current motion state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FAnim_MotionData CurrentMotion;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    UAnimSequence* CurrentAnimation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float CurrentAnimTime;

    // Motion matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TrajectoryWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 TrajectoryPredictionFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MatchThreshold;

    // Bone indices for matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Matching")
    TArray<int32> ImportantBoneIndices;

public:
    // Initialization
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData FindBestMotionMatch(const FVector& DesiredVelocity, const FVector& DesiredDirection);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionData& MotionData, const FVector& DesiredVelocity, const FVector& DesiredDirection);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentMotion(float DeltaTime);

    // Trajectory prediction
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    TArray<FVector> PredictTrajectory(const FVector& CurrentVelocity, const FVector& DesiredVelocity, float PredictionTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    TArray<FVector> CalculateBonePoses(UAnimSequence* Animation, float Time);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    TArray<FVector> CalculateBoneVelocities(UAnimSequence* Animation, float Time, float DeltaTime);

    // Animation blending
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToNewMotion(const FAnim_MotionData& NewMotion, float BlendTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateBlendWeight(float BlendTime, float MaxBlendTime);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddAnimationToDatabase(UAnimSequence* Animation, const FString& MotionType);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 GetDatabaseSize() const { return MotionDatabase.Num(); }

    // Getters
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    UAnimSequence* GetCurrentAnimation() const { return CurrentAnimation; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentAnimTime() const { return CurrentAnimTime; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotion() const { return CurrentMotion; }

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawTrajectory(const TArray<FVector>& Trajectory, const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintMotionInfo();

protected:
    // Internal helper functions
    float CalculatePoseDistance(const TArray<FVector>& Pose1, const TArray<FVector>& Pose2);
    float CalculateTrajectoryDistance(const TArray<FVector>& Trajectory1, const TArray<FVector>& Trajectory2);
    float CalculateVelocityDistance(const TArray<FVector>& Velocity1, const TArray<FVector>& Velocity2);
    
    void ExtractMotionDataFromAnimation(UAnimSequence* Animation, float StartTime, float EndTime, int32 SampleRate);
    void NormalizeMotionData(FAnim_MotionData& MotionData);
};