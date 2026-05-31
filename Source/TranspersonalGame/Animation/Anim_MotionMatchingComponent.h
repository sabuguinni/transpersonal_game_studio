#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/PoseAsset.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    float AngularVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    EAnim_MovementState MovementState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Frame")
    TArray<FTransform> BoneTransforms;

    FAnim_MotionFrame()
    {
        AnimSequence = nullptr;
        FrameTime = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        AngularVelocity = 0.0f;
        MovementState = EAnim_MovementState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionDatabase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionFrame> MotionFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FName> TrackedBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    float SamplingRate;

    FAnim_MotionDatabase()
    {
        SamplingRate = 30.0f; // 30 FPS sampling
        TrackedBones = {
            TEXT("pelvis"),
            TEXT("spine_01"),
            TEXT("spine_02"),
            TEXT("spine_03"),
            TEXT("head"),
            TEXT("upperarm_l"),
            TEXT("upperarm_r"),
            TEXT("lowerarm_l"),
            TEXT("lowerarm_r"),
            TEXT("hand_l"),
            TEXT("hand_r"),
            TEXT("thigh_l"),
            TEXT("thigh_r"),
            TEXT("calf_l"),
            TEXT("calf_r"),
            TEXT("foot_l"),
            TEXT("foot_r")
        };
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Core Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase(const TArray<UAnimSequence*>& AnimSequences);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionFrame FindBestMatch(const FVector& DesiredVelocity, const FVector& DesiredAcceleration, float DesiredAngularVelocity);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentMotion(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetDesiredMovement(const FVector& Velocity, const FVector& Acceleration, float AngularVel);

    // Database Management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void LoadMotionDatabase(const FString& DatabasePath);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SaveMotionDatabase(const FString& DatabasePath);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    // Query Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 GetDatabaseSize() const { return MotionDatabase.MotionFrames.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsMotionDatabaseValid() const { return MotionDatabase.MotionFrames.Num() > 0; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMatchingWeights(float VelocityWeight, float AccelerationWeight, float AngularWeight, float PoseWeight);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetBlendTime(float NewBlendTime) { BlendTime = FMath::Clamp(NewBlendTime, 0.1f, 2.0f); }

protected:
    // Motion Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    FAnim_MotionDatabase MotionDatabase;

    // Current Motion State
    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionFrame CurrentMotionFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionFrame TargetMotionFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    float CurrentBlendAlpha;

    // Desired Movement Input
    UPROPERTY(BlueprintReadOnly, Category = "Input")
    FVector DesiredVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    FVector DesiredAcceleration;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    float DesiredAngularVelocity;

    // Matching Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Config")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Config")
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Config")
    float AngularVelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Config")
    float PoseWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Config")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching Config")
    float MatchingThreshold;

    // Component References
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMesh;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

private:
    // Internal state
    float LastMatchTime;
    int32 CurrentFrameIndex;
    bool bIsBlending;

    // Helper functions
    void CacheComponentReferences();
    float CalculateMotionCost(const FAnim_MotionFrame& Frame, const FVector& DesiredVel, const FVector& DesiredAccel, float DesiredAngularVel) const;
    void ExtractMotionFrame(UAnimSequence* AnimSequence, float Time, FAnim_MotionFrame& OutFrame);
    FTransform GetBoneTransformAtTime(UAnimSequence* AnimSequence, const FName& BoneName, float Time) const;
    void BlendMotionFrames(const FAnim_MotionFrame& FrameA, const FAnim_MotionFrame& FrameB, float Alpha, FAnim_MotionFrame& OutFrame);
    void ApplyMotionFrame(const FAnim_MotionFrame& Frame);
    void LogMotionMatchingEvent(const FString& Event) const;
};