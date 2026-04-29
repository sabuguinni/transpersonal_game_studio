#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/PoseAsset.h"
#include "Engine/DataAsset.h"
#include "PrimitiveAnimationController.h"
#include "../SharedTypes.h"
#include "MotionMatchingComponent.generated.h"

// Motion matching pose data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PoseData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Data")
    FVector RootMotionVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Data")
    FVector RootMotionDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Data")
    float PoseTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Data")
    TArray<FVector> BonePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Data")
    TArray<FVector> BoneVelocities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Data")
    EAnim_MovementState MovementState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Data")
    float MatchingScore;

    FAnim_PoseData()
    {
        RootMotionVelocity = FVector::ZeroVector;
        RootMotionDirection = FVector::ForwardVector;
        PoseTime = 0.0f;
        MovementState = EAnim_MovementState::Idle;
        MatchingScore = 0.0f;
    }
};

// Motion matching database entry
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    TObjectPtr<UAnimSequence> AnimationSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    TArray<FAnim_PoseData> PoseDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    EAnim_MovementState AssociatedState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float AnimationLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsLooping;

    FAnim_MotionData()
    {
        AnimationSequence = nullptr;
        AssociatedState = EAnim_MovementState::Idle;
        AnimationLength = 0.0f;
        bIsLooping = true;
    }
};

// Motion matching search parameters
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SearchParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Parameters")
    FVector DesiredVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Parameters")
    FVector DesiredDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Parameters")
    EAnim_MovementState DesiredState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Parameters")
    float VelocityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Parameters")
    float DirectionWeight = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Parameters")
    float PoseWeight = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Parameters")
    float StateWeight = 2.0f;

    FAnim_SearchParams()
    {
        DesiredVelocity = FVector::ZeroVector;
        DesiredDirection = FVector::ForwardVector;
        DesiredState = EAnim_MovementState::Idle;
    }
};

/**
 * Motion Matching Component for realistic character animation
 * Implements pose-based animation selection for natural movement
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_PoseData FindBestMatchingPose(const FAnim_SearchParams& SearchParams);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatching(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildPoseDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionData(UAnimSequence* Animation, EAnim_MovementState State);

    // Pose comparison and scoring
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculatePoseScore(const FAnim_PoseData& PoseA, const FAnim_PoseData& PoseB, const FAnim_SearchParams& Params);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateVelocityScore(const FVector& VelA, const FVector& VelB);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateDirectionScore(const FVector& DirA, const FVector& DirB);

    // Database management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearPoseDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 GetPoseDatabaseSize() const;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_PoseData GetCurrentPose() const { return CurrentPose; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    UAnimSequence* GetCurrentAnimation() const { return CurrentAnimation; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentAnimationTime() const { return CurrentAnimationTime; }

protected:
    // Motion database
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Database")
    TArray<FAnim_MotionData> MotionDatabase;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FAnim_PoseData CurrentPose;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    TObjectPtr<UAnimSequence> CurrentAnimation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float CurrentAnimationTime;

    // Motion matching settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching Settings")
    float SearchRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching Settings")
    float MinimumMatchScore = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching Settings")
    int32 MaxSearchResults = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching Settings")
    float BlendTime = 0.2f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float UpdateFrequency = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bEnableDebugDrawing = false;

private:
    // Component references
    UPROPERTY()
    TObjectPtr<UPrimitiveAnimationController> AnimController;

    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    // Internal state
    float LastUpdateTime;
    float TimeSinceLastSearch;
    FAnim_SearchParams LastSearchParams;

    // Internal helper functions
    void ExtractPoseData(UAnimSequence* Animation, float Time, FAnim_PoseData& OutPoseData);
    void SampleAnimationAtTime(UAnimSequence* Animation, float Time, TArray<FVector>& OutBonePositions);
    void CalculateBoneVelocities(const TArray<FVector>& CurrentPositions, const TArray<FVector>& PreviousPositions, float DeltaTime, TArray<FVector>& OutVelocities);
    
    // Debug functions
    void DrawDebugPose(const FAnim_PoseData& Pose, FColor Color);
    void DrawDebugSearch(const FAnim_SearchParams& Params);
};