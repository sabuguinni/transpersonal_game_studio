#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnimationSystemManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAnimationSystem, Log, All);

/**
 * Motion Matching Data Structure
 * Stores pose data for real-time motion matching
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMotionMatchingPose
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector RootMotionVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector RootMotionDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FTransform> BoneTransforms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FName AnimationName;

    FMotionMatchingPose()
    {
        RootMotionVelocity = FVector::ZeroVector;
        RootMotionDirection = FVector::ForwardVector;
        TimeStamp = 0.0f;
        AnimationName = NAME_None;
    }
};

/**
 * Character Movement State for Animation
 */
UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Combat      UMETA(DisplayName = "Combat")
};

/**
 * IK Chain Configuration for procedural animation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FIKChainConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName StartBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName EndBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName EffectorBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableRotationLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float MaxRotationAngle;

    FIKChainConfig()
    {
        StartBone = NAME_None;
        EndBone = NAME_None;
        EffectorBone = NAME_None;
        BlendWeight = 1.0f;
        bEnableRotationLimit = true;
        MaxRotationAngle = 45.0f;
    }
};

/**
 * Animation System Manager
 * Core component responsible for advanced animation features:
 * - Motion Matching for fluid character movement
 * - Procedural IK for foot placement and interaction
 * - Animation blending and state management
 * - Performance optimization for large crowds
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion Matching System
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionMatching(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FMotionMatchingPose FindBestMatchingPose(const FVector& DesiredVelocity, const FVector& DesiredDirection);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingDatabase(UAnimSequence* AnimSequence);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToMotionMatchingPose(const FMotionMatchingPose& TargetPose, float BlendTime = 0.2f);

    // IK System
    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetupFootIK(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector PerformGroundTrace(const FVector& StartLocation, float TraceDistance = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKTarget(FName BoneName, const FVector& TargetLocation, const FRotator& TargetRotation);

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetMovementState(ECharacterMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    ECharacterMovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void StopAnimationMontage(UAnimMontage* Montage, float BlendOutTime = 0.25f);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAnimationLOD(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableCrowdOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateAnimationFrequency(float DistanceFromCamera);

protected:
    // Motion Matching Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FMotionMatchingPose> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MotionMatchingUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseSearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 MaxDatabaseSize;

    // IK Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    TArray<FIKChainConfig> IKChains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float GroundTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK;

    // Animation State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    ECharacterMovementState CurrentMovementState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    USkeletalMeshComponent* TargetSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    UAnimInstance* AnimInstance;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseCrowdOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AnimationUpdateDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentLODLevel;

private:
    // Internal Motion Matching
    float LastMotionMatchingUpdate;
    FMotionMatchingPose CurrentPose;
    FMotionMatchingPose TargetPose;
    
    // Internal IK Data
    TMap<FName, FVector> IKTargets;
    TMap<FName, FRotator> IKRotations;
    
    // Performance Tracking
    float LastPerformanceUpdate;
    float CurrentAnimationFrequency;

    // Helper Functions
    float CalculatePoseSimilarity(const FMotionMatchingPose& PoseA, const FMotionMatchingPose& PoseB);
    void ExtractPoseFromAnimation(UAnimSequence* AnimSequence, float TimeStamp, FMotionMatchingPose& OutPose);
    void ApplyIKToSkeleton(USkeletalMeshComponent* SkeletalMesh);
    void OptimizeAnimationForDistance(float Distance);
};