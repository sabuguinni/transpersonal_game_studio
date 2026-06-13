#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

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
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsJumping;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsCrouching = false;
        bIsJumping = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FAnim_MotionData MotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float PlayRate;

    FAnim_MotionClip()
    {
        AnimSequence = nullptr;
        BlendWeight = 0.0f;
        PlayRate = 1.0f;
    }
};

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionClip> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionClip BestMatchClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MatchThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimInstance* TargetAnimInstance;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(const FVector& InVelocity, bool bInIsMoving, bool bInIsCrouching, bool bInIsJumping);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionClip FindBestMatch(const FAnim_MotionData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionData& A, const FAnim_MotionData& B);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToClip(const FAnim_MotionClip& TargetClip);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* AnimSeq, const FAnim_MotionData& MotionData);

private:
    float CurrentBlendTime;
    bool bIsBlending;
    FAnim_MotionClip PreviousClip;
};