#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionPose
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FTransform> BonePoses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UAnimSequence* SourceAnimation;

    FAnim_MotionPose()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        TimeStamp = 0.0f;
        SourceAnimation = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionDatabase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionPose> Poses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<UAnimSequence*> SourceAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SamplingRate;

    FAnim_MotionDatabase()
    {
        SamplingRate = 30.0f; // 30 FPS sampling
    }
};

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

    // Motion matching database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionDatabase MotionDatabase;

    // Current character state
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector CurrentVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector CurrentAcceleration;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float CurrentSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float CurrentDirection;

    // Motion matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseWeight;

    // Blending parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    // Current best match
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    int32 BestMatchIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float MatchScore;

    // Animation functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase(const TArray<UAnimSequence*>& Animations);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 FindBestPoseMatch(const FVector& TargetVelocity, const FVector& TargetAcceleration, float TargetDirection);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculatePoseDistance(const FAnim_MotionPose& PoseA, const FAnim_MotionPose& PoseB);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCharacterState();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UAnimSequence* GetCurrentBestAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetCurrentAnimationTime() const;

private:
    // Internal state
    FVector PreviousVelocity;
    float DeltaTimeAccumulator;
    
    // Character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    // Animation component reference
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    // Helper functions
    void SampleAnimationPose(UAnimSequence* Animation, float Time, FAnim_MotionPose& OutPose);
    void ExtractMotionFeatures(const FAnim_MotionPose& Pose, FVector& OutVelocity, FVector& OutAcceleration);
    float CalculateFeatureDistance(const FVector& A, const FVector& B, float Weight);
};