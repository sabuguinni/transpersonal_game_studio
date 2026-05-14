#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Anim_MotionMatching.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UAnimSequence* Animation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TimeInAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector RootMotionVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector FuturePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FRotator FutureRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FVector> BonePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FVector> BoneVelocities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MatchCost;

    FAnim_MotionFrame()
    {
        Animation = nullptr;
        TimeInAnimation = 0.0f;
        RootMotionVelocity = FVector::ZeroVector;
        FuturePosition = FVector::ZeroVector;
        FutureRotation = FRotator::ZeroRotator;
        MatchCost = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionDatabase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionFrame> MotionFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<class UAnimSequence*> SourceAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SamplingRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FName> ImportantBones;

    FAnim_MotionDatabase()
    {
        SamplingRate = 30.0f; // 30 FPS sampling
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector DesiredVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector DesiredDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DesiredSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FVector> CurrentBonePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FVector> CurrentBoneVelocities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TrajectoryWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    FAnim_MotionQuery()
    {
        DesiredVelocity = FVector::ZeroVector;
        DesiredDirection = FVector::ForwardVector;
        DesiredSpeed = 0.0f;
        PoseWeight = 1.0f;
        TrajectoryWeight = 1.0f;
        VelocityWeight = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatching : public UObject
{
    GENERATED_BODY()

public:
    UAnim_MotionMatching();

    // Motion database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionDatabase MotionDatabase;

    // Matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseCostWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TrajectoryCostWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityCostWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MinSearchTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 MaxCandidates;

    // Current playback state
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class UAnimSequence* CurrentAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float CurrentAnimationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    int32 CurrentFrameIndex;

    // Main motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase(const TArray<UAnimSequence*>& Animations);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionFrame FindBestMatch(const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMatchCost(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentFrame(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool ShouldSearchForNewMatch(const FAnim_MotionQuery& Query);

    // Database management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddAnimationToDatabase(UAnimSequence* Animation);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 GetDatabaseSize() const;

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FVector GetCurrentRootMotionVelocity() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentAnimationProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetImportantBones(const TArray<FName>& BoneNames);

protected:
    // Internal functions
    void SampleAnimationFrame(UAnimSequence* Animation, float Time, FAnim_MotionFrame& OutFrame);
    float CalculatePoseCost(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);
    float CalculateTrajectoryCost(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);
    float CalculateVelocityCost(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);
    
    TArray<FVector> ExtractBonePositions(UAnimSequence* Animation, float Time);
    TArray<FVector> ExtractBoneVelocities(UAnimSequence* Animation, float Time);
    FVector CalculateFuturePosition(UAnimSequence* Animation, float Time, float PredictionTime);
    FRotator CalculateFutureRotation(UAnimSequence* Animation, float Time, float PredictionTime);

private:
    // Search optimization
    float LastSearchTime;
    FAnim_MotionQuery LastQuery;
    
    // Performance tracking
    int32 FramesProcessed;
    float TotalSearchTime;
};