#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "SharedTypes.h"
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
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsJumping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching;

    FAnim_MotionFeature()
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
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionFeature> Features;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 ClipID;

    FAnim_MotionClip()
    {
        AnimSequence = nullptr;
        StartTime = 0.0f;
        EndTime = 0.0f;
        ClipID = -1;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionClip> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bEnableMotionMatching;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FAnim_MotionFeature CurrentFeatures;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 CurrentClipIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentPlayTime;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentFeatures(const FVector& InVelocity, const FVector& InAcceleration, bool bInIsJumping, bool bInIsCrouching);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 FindBestMatchingClip(const FAnim_MotionFeature& TargetFeatures);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* AnimSeq, const TArray<FAnim_MotionFeature>& ClipFeatures);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UAnimSequence* GetCurrentAnimSequence() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateFeatureDistance(const FAnim_MotionFeature& FeatureA, const FAnim_MotionFeature& FeatureB) const;

private:
    void InitializeMotionDatabase();
    void ProcessMotionMatching(float DeltaTime);
    
    float LastUpdateTime;
    bool bDatabaseInitialized;
};