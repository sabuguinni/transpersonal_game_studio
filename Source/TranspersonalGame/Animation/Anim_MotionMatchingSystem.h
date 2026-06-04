#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/PoseAsset.h"
#include "Engine/DataAsset.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

class UAnimSequence;
class UPoseAsset;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UAnimSequence> Animation;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    float TimeInAnimation = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    FVector RootMotionVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    FVector FuturePosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    FRotator FutureRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    TArray<FTransform> BoneTransforms;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    float MatchCost = 0.0f;

    FAnim_MotionFrame()
    {
        Animation = nullptr;
        TimeInAnimation = 0.0f;
        RootMotionVelocity = FVector::ZeroVector;
        FuturePosition = FVector::ZeroVector;
        FutureRotation = FRotator::ZeroRotator;
        MatchCost = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionQuery
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    FVector DesiredVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    FVector DesiredFacing = FVector::ForwardVector;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    EAnim_MovementMode MovementMode = EAnim_MovementMode::Walking;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    EAnim_Stance Stance = EAnim_Stance::Relaxed;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    float ResponseTime = 0.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Motion Matching")
    bool bRequireRootMotion = true;

    FAnim_MotionQuery()
    {
        DesiredVelocity = FVector::ZeroVector;
        DesiredFacing = FVector::ForwardVector;
        MovementMode = EAnim_MovementMode::Walking;
        Stance = EAnim_Stance::Relaxed;
        ResponseTime = 0.5f;
        bRequireRootMotion = true;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAnim_MotionDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<TObjectPtr<UAnimSequence>> MotionAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionFrame> PrecomputedFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    float FrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    float PredictionTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FName> ImportantBones;

    UAnim_MotionDatabase()
    {
        FrameRate = 30.0f;
        PredictionTime = 0.5f;
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

    // Motion Matching Core Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionFrame FindBestMatch(const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionDatabase(UAnim_MotionDatabase* NewDatabase);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentMotion(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void RequestMotionTransition(const FAnim_MotionQuery& NewQuery);

    // Motion Analysis
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionCost(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PrecomputeMotionDatabase();

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionFrame GetCurrentMotionFrame() const { return CurrentMotionFrame; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsTransitioning() const { return bIsTransitioning; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetTransitionProgress() const { return TransitionProgress; }

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UAnim_MotionDatabase> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DirectionWeight = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseWeight = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float FutureWeight = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TransitionDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bEnableMotionMatching = true;

private:
    UPROPERTY()
    FAnim_MotionFrame CurrentMotionFrame;

    UPROPERTY()
    FAnim_MotionFrame TargetMotionFrame;

    UPROPERTY()
    FAnim_MotionQuery CurrentQuery;

    UPROPERTY()
    bool bIsTransitioning = false;

    UPROPERTY()
    float TransitionProgress = 0.0f;

    UPROPERTY()
    float TransitionTimer = 0.0f;

    // Internal Methods
    void InitializeMotionMatching();
    FAnim_MotionFrame ExtractMotionFrame(UAnimSequence* Animation, float Time);
    float CalculateVelocityCost(const FVector& FrameVelocity, const FVector& QueryVelocity);
    float CalculateDirectionCost(const FVector& FrameFacing, const FVector& QueryFacing);
    float CalculatePoseCost(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);
    float CalculateFutureCost(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);
    void UpdateTransition(float DeltaTime);
    void ApplyMotionFrame(const FAnim_MotionFrame& Frame, float BlendWeight = 1.0f);
};