#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStamp;

    FAnim_MotionFrame()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsGrounded = true;
        TimeStamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAnim_MotionFrame> MotionFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ClipDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClipName;

    FAnim_MotionClip()
    {
        AnimSequence = nullptr;
        ClipDuration = 0.0f;
        ClipName = TEXT("DefaultClip");
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UAnimSequence* FindBestMatchingAnimation(const FAnim_MotionFrame& CurrentFrame, const FAnim_MotionFrame& PredictedFrame);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* AnimSequence, const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionFrame GetCurrentMotionFrame() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionFrame PredictFutureMotionFrame(float PredictionTime) const;

protected:
    // Motion clip database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionClip> MotionClips;

    // Motion matching settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float VelocityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float AccelerationWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float DirectionWeight = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float SpeedWeight = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float PredictionTime = 0.3f;

    // Current motion state
    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionFrame CurrentMotionFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionFrame PreviousMotionFrame;

    // Character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter;

private:
    float CalculateMotionScore(const FAnim_MotionFrame& TargetFrame, const FAnim_MotionFrame& ClipFrame) const;
    void UpdateMotionFrame();
    void ExtractMotionDataFromAnimation(UAnimSequence* AnimSequence, FAnim_MotionClip& OutClip);

    // Frame history for prediction
    TArray<FAnim_MotionFrame> FrameHistory;
    static const int32 MaxFrameHistory = 10;
};