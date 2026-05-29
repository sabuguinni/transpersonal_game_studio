#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector RootPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FRotator RootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    TArray<FTransform> BoneTransforms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FString AnimationName;

    FAnim_MotionFrame()
    {
        RootPosition = FVector::ZeroVector;
        RootRotation = FRotator::ZeroRotator;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        TimeStamp = 0.0f;
        AnimationName = TEXT("None");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionDatabase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<FAnim_MotionFrame> MotionFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<UAnimSequence*> SourceAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    int32 FramesPerSecond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    float DatabaseDuration;

    FAnim_MotionDatabase()
    {
        FramesPerSecond = 30;
        DatabaseDuration = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    FVector DesiredVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    FVector DesiredDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float DesiredSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    FVector CurrentPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    FRotator CurrentRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    TArray<FString> PreferredAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float PositionWeight;

    FAnim_MotionQuery()
    {
        DesiredVelocity = FVector::ZeroVector;
        DesiredDirection = FVector::ForwardVector;
        DesiredSpeed = 0.0f;
        CurrentPosition = FVector::ZeroVector;
        CurrentRotation = FRotator::ZeroRotator;
        VelocityWeight = 1.0f;
        DirectionWeight = 0.5f;
        PositionWeight = 0.3f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionDatabase MotionDatabase;

    // Current query
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionQuery CurrentQuery;

    // Current best match
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    int32 CurrentBestFrameIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float CurrentMatchScore;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinimumMatchThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDebugDrawing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateFrequency;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bIsInitialized;

public:
    // Main motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 FindBestMotionMatch(const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionQuery(FVector DesiredVelocity, FVector CurrentPosition, FRotator CurrentRotation);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionFrame GetMotionFrame(int32 FrameIndex);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsValidFrameIndex(int32 FrameIndex);

    // Database management
    UFUNCTION(BlueprintCallable, Category = "Database")
    void BuildMotionDatabase(const TArray<UAnimSequence*>& Animations);

    UFUNCTION(BlueprintCallable, Category = "Database")
    void AddAnimationToDatabase(UAnimSequence* Animation);

    UFUNCTION(BlueprintCallable, Category = "Database")
    void ClearMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Database")
    int32 GetDatabaseSize() const { return MotionDatabase.MotionFrames.Num(); }

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SetMotionMatchingEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Utility")
    bool IsMotionMatchingEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugMotionData();

private:
    void ExtractMotionFramesFromAnimation(UAnimSequence* Animation);
    float CalculateVelocityScore(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);
    float CalculateDirectionScore(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);
    float CalculatePositionScore(const FAnim_MotionFrame& Frame, const FAnim_MotionQuery& Query);
    void UpdateInternalState(float DeltaTime);

    bool bMotionMatchingEnabled;
    float AccumulatedTime;
};