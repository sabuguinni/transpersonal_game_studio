#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsFalling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsJumping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Acceleration;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsFalling = false;
        bIsJumping = false;
        Acceleration = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PoseCandidate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    class UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    float MatchScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    FAnim_MotionData PoseMotionData;

    FAnim_PoseCandidate()
    {
        AnimSequence = nullptr;
        StartTime = 0.0f;
        MatchScore = 0.0f;
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

    // Motion Matching Core Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_PoseCandidate FindBestMatchingPose(const FAnim_MotionData& CurrentMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToPose(const FAnim_PoseCandidate& TargetPose, float BlendTime = 0.2f);

    // Animation Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Database")
    TArray<class UAnimSequence*> IdleAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Database")
    TArray<class UAnimSequence*> WalkAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Database")
    TArray<class UAnimSequence*> RunAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Database")
    TArray<class UAnimSequence*> JumpAnimations;

    // Motion Matching Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0"))
    float MinMatchThreshold;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FAnim_PoseCandidate CurrentPose;

    // Component References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class USkeletalMeshComponent* SkeletalMeshComponent;

private:
    // Internal Functions
    float CalculatePoseMatchScore(const FAnim_MotionData& CurrentMotion, const FAnim_MotionData& PoseMotion);
    void CacheComponentReferences();
    TArray<FAnim_PoseCandidate> BuildPoseCandidates();

    // Cached Data
    TArray<FAnim_PoseCandidate> CachedPoseCandidates;
    float LastUpdateTime;
    bool bIsInitialized;
};