#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
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
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MovementState MovementState;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PoseMatchData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    FName PoseName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    float MatchScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    TArray<FTransform> BoneTransforms;

    FAnim_PoseMatchData()
    {
        PoseName = NAME_None;
        MatchScore = 0.0f;
        TimeStamp = 0.0f;
        BoneTransforms.Empty();
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion analysis functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData AnalyzeCurrentMotion();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_PoseMatchData FindBestPoseMatch(const FAnim_MotionData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionDatabase(const FAnim_MotionData& NewMotionData);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionSimilarity(const FAnim_MotionData& MotionA, const FAnim_MotionData& MotionB);

    // Animation blending functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToTargetPose(const FAnim_PoseMatchData& TargetPose, float BlendTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsMotionMatchingActive() const { return bMotionMatchingEnabled; }

protected:
    // Motion database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionData> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_PoseMatchData> PoseDatabase;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MotionSampleRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PoseMatchThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BlendSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxDatabaseEntries;

    // State tracking
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FAnim_MotionData CurrentMotion;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FAnim_PoseMatchData CurrentBestMatch;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bMotionMatchingEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastSampleTime;

private:
    // Internal functions
    void SampleCurrentMotion();
    void CleanupOldEntries();
    FVector GetOwnerVelocity() const;
    bool IsOwnerInAir() const;
    bool IsOwnerCrouching() const;
};