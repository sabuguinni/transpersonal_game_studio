#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Anim_MotionMatchingController.generated.h"

USTRUCT(BlueprintType)
struct FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsTurning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float TurnRate = 0.0f;

    FAnim_MotionFrame()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsTurning = false;
        TurnRate = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_AnimationClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* AnimSequence = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FString ClipName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MinSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MaxSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsLooping = true;

    FAnim_AnimationClip()
    {
        AnimSequence = nullptr;
        ClipName = TEXT("");
        MinSpeed = 0.0f;
        MaxSpeed = 100.0f;
        Weight = 1.0f;
        bIsLooping = true;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatching(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_AnimationClip FindBestMatchingAnimation(const FAnim_MotionFrame& CurrentMotion) const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayMatchedAnimation(const FAnim_AnimationClip& AnimClip);

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionFrame GetCurrentMotionFrame() const { return CurrentMotionFrame; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddAnimationClip(UAnimSequence* AnimSequence, const FString& ClipName, float MinSpeed, float MaxSpeed);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingEnabled(bool bEnabled) { bMotionMatchingEnabled = bEnabled; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionFrame CurrentMotionFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionFrame PreviousMotionFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Clips")
    TArray<FAnim_AnimationClip> AnimationClips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bMotionMatchingEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MotionSmoothingFactor = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinMovementThreshold = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TurnRateThreshold = 30.0f;

    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

private:
    UFUNCTION()
    void UpdateMotionFrame(float DeltaTime);

    UFUNCTION()
    float CalculateAnimationScore(const FAnim_AnimationClip& AnimClip, const FAnim_MotionFrame& MotionFrame) const;

    UFUNCTION()
    void InitializeDefaultAnimations();
};