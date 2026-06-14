#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "../Core/SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

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
    bool bIsGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float GroundDistance;

    FAnim_MotionFeature()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsGrounded = true;
        GroundDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionFeature> Features;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FString ClipName;

    FAnim_MotionClip()
    {
        AnimSequence = nullptr;
        StartTime = 0.0f;
        EndTime = 0.0f;
        ClipName = TEXT("Default");
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionClip> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float FeatureWeight_Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float FeatureWeight_Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float FeatureWeight_Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FAnim_MotionFeature CurrentFeatures;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 CurrentClipIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentClipTime;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentFeatures(const FVector& InVelocity, const FVector& InAcceleration, bool bInIsGrounded);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 FindBestMatchingClip(const FAnim_MotionFeature& TargetFeatures);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateFeatureDistance(const FAnim_MotionFeature& A, const FAnim_MotionFeature& B);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* AnimSeq, const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UAnimSequence* GetCurrentAnimation();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetFeatureWeights(float VelWeight, float SpeedWeight, float DirWeight);

private:
    void ExtractFeaturesFromAnimation(UAnimSequence* AnimSeq, FAnim_MotionClip& OutClip);
    void SampleAnimationFeatures(UAnimSequence* AnimSeq, float SampleTime, FAnim_MotionFeature& OutFeature);
};