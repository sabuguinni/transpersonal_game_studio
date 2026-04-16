#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/PoseAsset.h"
#include "GameFramework/Character.h"
#include "../SharedTypes.h"
#include "MotionMatchingSystem.generated.h"

// Motion matching pose data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PoseData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    FVector RootMotionVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    FVector RootMotionAcceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    TArray<FTransform> BonePoses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    float MatchingScore;

    FAnim_PoseData()
    {
        RootMotionVelocity = FVector::ZeroVector;
        RootMotionAcceleration = FVector::ZeroVector;
        TimeStamp = 0.0f;
        MatchingScore = 0.0f;
    }
};

// Motion matching feature weights
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FeatureWeights
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float PoseWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float TrajectoryWeight;

    FAnim_FeatureWeights()
    {
        VelocityWeight = 1.0f;
        AccelerationWeight = 0.5f;
        PoseWeight = 2.0f;
        TrajectoryWeight = 1.5f;
    }
};

// Motion matching database entry
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    TSoftObjectPtr<UAnimSequence> AnimationSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    TArray<FAnim_PoseData> PoseDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FString MotionTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bLooping;

    FAnim_MotionData()
    {
        MotionTag = TEXT("Default");
        PlayRate = 1.0f;
        bLooping = false;
    }
};

/**
 * Motion Matching System
 * Provides fluid character animation by matching current character state to a database of poses
 * Enables natural movement transitions for prehistoric survival gameplay
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core motion matching
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatching(const FVector& DesiredVelocity, const FVector& CurrentVelocity);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_PoseData FindBestMatchingPose(const FVector& TargetVelocity, const FVector& TargetAcceleration);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildPoseDatabase();

    // Database management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionData(UAnimSequence* Animation, const FString& Tag, bool bIsLooping = false);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    int32 GetDatabaseSize() const { return MotionDatabase.Num(); }

    // Feature extraction
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_PoseData ExtractPoseFeatures(UAnimSequence* Animation, float TimeStamp);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    TArray<FVector> PredictTrajectory(const FVector& CurrentVelocity, const FVector& DesiredVelocity, float PredictionTime = 1.0f);

    // Scoring and matching
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float CalculatePoseScore(const FAnim_PoseData& PoseA, const FAnim_PoseData& PoseB);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetFeatureWeights(const FAnim_FeatureWeights& NewWeights);

    // Current state
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_PoseData GetCurrentPose() const { return CurrentPose; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    UAnimSequence* GetCurrentAnimation() const { return CurrentAnimation; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentPlayTime() const { return CurrentPlayTime; }

protected:
    // Motion database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionData> MotionDatabase;

    // Feature weights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_FeatureWeights FeatureWeights;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FAnim_PoseData CurrentPose;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    UAnimSequence* CurrentAnimation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float CurrentPlayTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    int32 CurrentMotionIndex;

    // Matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Settings")
    float MatchingThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Settings")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Settings")
    int32 MaxSearchResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Settings")
    bool bEnableDebugDraw;

    // Trajectory prediction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    float TrajectoryPredictionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
    int32 TrajectorySteps;

    // References
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

private:
    // Internal matching logic
    void UpdateCurrentPose();
    void ProcessMotionMatching(const FVector& DesiredVelocity);
    
    // Feature extraction helpers
    FVector ExtractRootMotionVelocity(UAnimSequence* Animation, float Time);
    FVector ExtractRootMotionAcceleration(UAnimSequence* Animation, float Time);
    TArray<FTransform> ExtractBonePoses(UAnimSequence* Animation, float Time);

    // Scoring helpers
    float ScoreVelocityMatch(const FVector& VelA, const FVector& VelB);
    float ScoreAccelerationMatch(const FVector& AccelA, const FVector& AccelB);
    float ScorePoseMatch(const TArray<FTransform>& PoseA, const TArray<FTransform>& PoseB);
    float ScoreTrajectoryMatch(const TArray<FVector>& TrajA, const TArray<FVector>& TrajB);

    // Database helpers
    void SampleAnimationPoses(FAnim_MotionData& MotionData, float SampleRate = 30.0f);
    bool IsValidAnimationSequence(UAnimSequence* Animation);

    // Debug
    void DrawDebugInfo();
    void LogMatchingResults(const FAnim_PoseData& BestMatch, float Score);
};