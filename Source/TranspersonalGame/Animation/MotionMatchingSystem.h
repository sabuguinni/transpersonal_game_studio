#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/PoseAsset.h"
#include "../SharedTypes.h"
#include "MotionMatchingSystem.generated.h"

// Forward declarations
class USkeletalMeshComponent;
class UAnimationStateManager;

USTRUCT(BlueprintType)
struct FAnim_MotionFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Weight;

    FAnim_MotionFeature()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Weight = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionFeature> BoneFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector RootMotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector MovementDirection;

    FAnim_MotionFrame()
    {
        TimeStamp = 0.0f;
        RootMotion = FVector::ZeroVector;
        Speed = 0.0f;
        MovementDirection = FVector::ForwardVector;
    }
};

USTRUCT(BlueprintType)
struct FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UAnimSequence* AnimationSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionFrame> MotionFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FString ClipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float ClipDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsLooping;

    FAnim_MotionClip()
    {
        AnimationSequence = nullptr;
        ClipName = TEXT("DefaultClip");
        ClipDuration = 0.0f;
        bIsLooping = false;
    }
};

USTRUCT(BlueprintType)
struct FAnim_MotionQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionFeature> DesiredFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DesiredSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector DesiredDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MinimumMatchThreshold;

    FAnim_MotionQuery()
    {
        DesiredSpeed = 0.0f;
        DesiredDirection = FVector::ForwardVector;
        SearchRadius = 100.0f;
        MinimumMatchThreshold = 0.7f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_MotionMatch
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 ClipIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 FrameIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MatchScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendWeight;

    FAnim_MotionMatch()
    {
        ClipIndex = -1;
        FrameIndex = -1;
        MatchScore = 0.0f;
        BlendWeight = 0.0f;
    }
};

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
    // Motion matching core functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionMatch FindBestMatch(const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    TArray<FAnim_MotionMatch> FindMultipleMatches(const FAnim_MotionQuery& Query, int32 MaxMatches = 3);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayMotionMatch(const FAnim_MotionMatch& Match, float BlendTime = 0.2f);

    // Database management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* Animation, const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void RemoveMotionClip(const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

    // Feature extraction
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ExtractFeaturesFromAnimation(UAnimSequence* Animation, TArray<FAnim_MotionFrame>& OutFrames);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionFeature ExtractBoneFeature(const FTransform& BoneTransform, const FTransform& PreviousBoneTransform, float DeltaTime);

    // Query building
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionQuery BuildQueryFromCurrentState(float DesiredSpeed, const FVector& DesiredDirection);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentMotionFeatures();

    // Settings and configuration
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetFeatureBones(const TArray<FName>& BoneNames);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMatchingWeights(float PositionWeight, float VelocityWeight, float RotationWeight);

protected:
    // Motion database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionClip> MotionClips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FName> FeatureBones;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    TArray<FAnim_MotionFeature> CurrentFeatures;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FAnim_MotionMatch CurrentMatch;

    // Matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Parameters")
    float PositionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Parameters")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Parameters")
    float RotationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Parameters")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Parameters")
    float SearchWindow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Parameters")
    bool bUseRootMotion;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    UAnimationStateManager* AnimationStateManager;

    // Performance optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSearchFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

private:
    // Internal matching functions
    float CalculateMatchScore(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);
    float CalculateFeatureDistance(const FAnim_MotionFeature& Feature1, const FAnim_MotionFeature& Feature2);
    void InitializeDefaultFeatureBones();
    
    // Update timing
    float LastUpdateTime;
    float UpdateTimer;
};