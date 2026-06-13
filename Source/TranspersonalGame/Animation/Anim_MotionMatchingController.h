#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TurnRate;

    FAnim_MotionMatchingData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        TurnRate = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    class UAnimSequence* AnimationSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    FAnim_MotionMatchingData MotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    float MatchingScore;

    FAnim_MotionClip()
    {
        AnimationSequence = nullptr;
        StartTime = 0.0f;
        EndTime = 0.0f;
        MatchingScore = 0.0f;
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

    // Motion matching database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionClip> MotionDatabase;

    // Current motion state
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionMatchingData CurrentMotionData;

    // Best matching clip
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionClip BestMatchingClip;

    // Motion matching settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float TurnRateWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float MinimumMatchingThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float BlendTime;

    // Animation blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    class UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    class UAnimMontage* LandingMontage;

    // Character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* CharacterMesh;

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionClip FindBestMatchingClip(const FAnim_MotionMatchingData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionMatchingData& ClipMotion, const FAnim_MotionMatchingData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayMatchingAnimation(const FAnim_MotionClip& MotionClip);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClipToDatabase(UAnimSequence* Animation, float StartTime, float EndTime, const FAnim_MotionMatchingData& MotionData);

    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayLandingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementBlendSpaceValues(float Speed, float Direction);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsMotionMatchingActive() const;

private:
    // Internal state
    bool bMotionMatchingEnabled;
    float LastUpdateTime;
    float MotionUpdateInterval;

    // Animation state tracking
    bool bIsPlayingMontage;
    float CurrentMontageTime;

    // Performance optimization
    int32 DatabaseSearchIndex;
    float LastDatabaseUpdateTime;
    float DatabaseUpdateInterval;
};