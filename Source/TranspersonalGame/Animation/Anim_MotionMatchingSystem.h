#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

// Motion matching pose data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionPose
{
    GENERATED_BODY()

    // Pose identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 PoseID;

    // Animation sequence reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UAnimSequence* AnimSequence;

    // Time within the animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float AnimTime;

    // Velocity at this pose
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity;

    // Acceleration at this pose
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Acceleration;

    // Future trajectory points
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FVector> FutureTrajectory;

    // Past trajectory points
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FVector> PastTrajectory;

    // Bone positions for key joints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FVector> KeyBonePositions;

    // Bone velocities for key joints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FVector> KeyBoneVelocities;

    // Motion tags (walking, running, crouching, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FName> MotionTags;

    FAnim_MotionPose()
    {
        PoseID = -1;
        AnimSequence = nullptr;
        AnimTime = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
    }
};

// Motion matching query structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionQuery
{
    GENERATED_BODY()

    // Current velocity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    FVector CurrentVelocity;

    // Desired velocity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    FVector DesiredVelocity;

    // Current acceleration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    FVector CurrentAcceleration;

    // Desired trajectory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    TArray<FVector> DesiredTrajectory;

    // Required motion tags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    TArray<FName> RequiredTags;

    // Forbidden motion tags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    TArray<FName> ForbiddenTags;

    // Weight factors for matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    float TrajectoryWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Query")
    float PoseWeight;

    FAnim_MotionQuery()
    {
        CurrentVelocity = FVector::ZeroVector;
        DesiredVelocity = FVector::ZeroVector;
        CurrentAcceleration = FVector::ZeroVector;
        VelocityWeight = 1.0f;
        TrajectoryWeight = 1.0f;
        PoseWeight = 1.0f;
    }
};

/**
 * Motion Matching System for realistic character animation
 * Implements advanced motion matching techniques for fluid character movement
 * Based on research from "Motion Matching and The Road to Next-Gen Animation"
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion matching database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionPose> MotionDatabase;

    // Current best matching pose
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionPose CurrentPose;

    // Animation blending settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    int32 MaxSearchResults;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableMultithreading;

    // Debug visualization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowTrajectory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowBonePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowMatchingScore;

    // Core motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionPose FindBestMatch(const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMatchingScore(const FAnim_MotionPose& Pose, const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase(const TArray<class UAnimSequence*>& AnimSequences);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentQuery(const FVector& DesiredVelocity, const TArray<FVector>& DesiredTrajectory);

    // Trajectory prediction
    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    TArray<FVector> PredictTrajectory(const FVector& CurrentVelocity, const FVector& DesiredVelocity, float PredictionTime, int32 SampleCount);

    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    void UpdateTrajectoryHistory(const FVector& CurrentPosition);

    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void TransitionToPose(const FAnim_MotionPose& NewPose);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    bool CanTransitionToPose(const FAnim_MotionPose& NewPose);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugTrajectory(const TArray<FVector>& Trajectory, FColor Color, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugPose(const FAnim_MotionPose& Pose, FColor Color, float Duration);

private:
    // Internal state
    FAnim_MotionQuery CurrentQuery;
    TArray<FVector> TrajectoryHistory;
    float LastUpdateTime;
    bool bIsInitialized;

    // Character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    // Internal helper functions
    void InitializeComponent();
    void UpdateMotionQuery(float DeltaTime);
    float CalculateVelocityScore(const FAnim_MotionPose& Pose, const FAnim_MotionQuery& Query);
    float CalculateTrajectoryScore(const FAnim_MotionPose& Pose, const FAnim_MotionQuery& Query);
    float CalculatePoseScore(const FAnim_MotionPose& Pose, const FAnim_MotionQuery& Query);
    bool MatchesTags(const FAnim_MotionPose& Pose, const FAnim_MotionQuery& Query);
    void ExtractPoseData(UAnimSequence* AnimSequence, float Time, FAnim_MotionPose& OutPose);
};