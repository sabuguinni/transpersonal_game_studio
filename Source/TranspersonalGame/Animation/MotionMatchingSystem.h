#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Engine/DataAsset.h"
#include "MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct FMotionFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Feature")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Feature")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Feature")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Feature")
    float Weight;

    FMotionFeature()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Weight = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FMotionFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    TArray<FMotionFeature> Features;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    FVector RootMotionDelta;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    FRotator RootRotationDelta;

    FMotionFrame()
    {
        TimeStamp = 0.0f;
        RootMotionDelta = FVector::ZeroVector;
        RootRotationDelta = FRotator::ZeroRotator;
    }
};

USTRUCT(BlueprintType)
struct FMotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    TObjectPtr<UAnimSequence> AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    TArray<FMotionFrame> Frames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    FString ClipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    float ClipLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    bool bIsLooping;

    FMotionClip()
    {
        AnimSequence = nullptr;
        ClipName = TEXT("");
        ClipLength = 0.0f;
        bIsLooping = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMotionDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FMotionClip> MotionClips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    int32 FeatureCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    float FrameRate;

    UMotionDatabase()
    {
        FeatureCount = 6; // Position, Velocity, Rotation for 2 trajectory points
        FrameRate = 30.0f;
    }

    UFUNCTION(BlueprintCallable, Category = "Motion Database")
    int32 FindBestMatch(const TArray<FMotionFeature>& QueryFeatures, float& OutScore) const;

    UFUNCTION(BlueprintCallable, Category = "Motion Database")
    void BuildDatabase();
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UMotionDatabase> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 TrajectoryPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TrajectoryTimeHorizon;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FMotionClip CurrentClip;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    int32 CurrentFrameIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float CurrentPlayTime;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateTrajectory(const FVector& DesiredVelocity, const FRotator& DesiredRotation);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FMotionClip FindBestMotionMatch();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToNewMotion(const FMotionClip& NewClip);

    UFUNCTION(BlueprintImplementableEvent, Category = "Motion Matching")
    void OnMotionChanged(const FMotionClip& NewClip);

private:
    TArray<FMotionFeature> CurrentTrajectory;
    TArray<FMotionFeature> PredictedTrajectory;
    
    float CalculateFeatureDistance(const FMotionFeature& A, const FMotionFeature& B) const;
    void PredictTrajectory(const FVector& DesiredVelocity, const FRotator& DesiredRotation);
    void ExtractCurrentFeatures();
};