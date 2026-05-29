#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsMoving;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimSequence> AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FAnim_MotionData MotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    float QualityScore;

    FAnim_MotionClip()
    {
        StartTime = 0.0f;
        EndTime = 1.0f;
        QualityScore = 1.0f;
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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionClip> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bEnableMotionMatching;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FAnim_MotionClip CurrentBestMatch;

public:
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(const FVector& InVelocity, const FVector& InAcceleration);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionClip FindBestMatch(const FAnim_MotionData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* AnimSequence, const FAnim_MotionData& MotionData, float StartTime = 0.0f, float EndTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionData& A, const FAnim_MotionData& B) const;

private:
    void InitializeDefaultMotions();
    float CalculateVelocityScore(const FVector& A, const FVector& B) const;
    float CalculateDirectionScore(float A, float B) const;
};