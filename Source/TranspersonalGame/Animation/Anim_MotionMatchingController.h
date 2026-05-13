#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingController.generated.h"

class UAnimSequence;
class UBlendSpace1D;
class UBlendSpace;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionFeature
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
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching;

    FAnim_MotionFeature()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionFeature MotionFeature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Quality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_MovementState MovementState;

    FAnim_MotionClip()
    {
        AnimSequence = nullptr;
        StartTime = 0.0f;
        EndTime = 0.0f;
        Quality = 1.0f;
        MovementState = EAnim_MovementState::Idle;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionClip> MotionDatabase;

    // Current Motion State
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionFeature CurrentMotionFeature;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionClip CurrentMotionClip;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float CurrentPlayTime;

    // Motion Matching Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Settings")
    float FeatureMatchThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Settings")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Settings")
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Settings")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Settings")
    float MinClipDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching|Settings")
    bool bEnableMotionMatching;

    // Character Movement Reference
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class USkeletalMeshComponent* MeshComponent;

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionFeature(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionClip FindBestMotionMatch(const FAnim_MotionFeature& TargetFeature);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionDistance(const FAnim_MotionFeature& FeatureA, const FAnim_MotionFeature& FeatureB);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void TransitionToMotionClip(const FAnim_MotionClip& NewClip);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClipToDatabase(UAnimSequence* AnimSequence, float StartTime, float EndTime, const FAnim_MotionFeature& Feature, EAnim_MovementState MovementState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    // Animation Blending
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetBlendWeight(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetCurrentBlendWeight() const;

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching|Debug")
    void DebugDrawMotionFeature(const FAnim_MotionFeature& Feature, FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching|Debug")
    void PrintMotionDatabaseInfo();

private:
    // Internal state
    float BlendWeight;
    float LastUpdateTime;
    FVector PreviousVelocity;
    bool bIsInitialized;

    // Helper functions
    void InitializeComponents();
    void ValidateMotionDatabase();
    bool IsValidMotionClip(const FAnim_MotionClip& Clip) const;
    void SmoothMotionFeature(FAnim_MotionFeature& Feature, float SmoothingFactor);
};