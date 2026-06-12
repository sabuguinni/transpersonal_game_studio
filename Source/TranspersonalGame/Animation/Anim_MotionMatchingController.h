#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Anim_MotionMatchingController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
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
    bool bIsJumping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsJumping = false;
        bIsCrouching = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UAnimSequence* AnimationSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionData> MotionFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float ClipLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FString ClipName;

    FAnim_MotionClip()
    {
        AnimationSequence = nullptr;
        ClipLength = 0.0f;
        ClipName = TEXT("DefaultClip");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UObject
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

    // Motion Matching Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionClip> MotionDatabase;

    // Current Motion State
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData CurrentMotionData;

    // Motion Matching Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PositionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionClip FindBestMatchingClip(const FAnim_MotionData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionData& A, const FAnim_MotionData& B);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* Animation, const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    // Animation Playback
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayMotionClip(class UAnimInstance* AnimInstance, const FAnim_MotionClip& Clip);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching", CallInEditor)
    void DebugPrintMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching", CallInEditor)
    void ValidateMotionDatabase();

protected:
    // Internal motion analysis
    void AnalyzeAnimationSequence(UAnimSequence* Animation, FAnim_MotionClip& OutClip);
    
    // Motion prediction
    FAnim_MotionData PredictNextMotion(const FAnim_MotionData& Current, float DeltaTime);
    
    // Database optimization
    void OptimizeDatabase();
};