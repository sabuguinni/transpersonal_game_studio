#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/PoseAsset.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsJumping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsFalling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsCrouching;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsJumping = false;
        bIsFalling = false;
        bIsCrouching = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PoseCandidate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Candidate")
    class UPoseAsset* PoseAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Candidate")
    float PoseTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Candidate")
    float MatchCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Candidate")
    FAnim_MotionData PoseMotionData;

    FAnim_PoseCandidate()
    {
        PoseAsset = nullptr;
        PoseTime = 0.0f;
        MatchCost = FLT_MAX;
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

    // Motion Matching Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<class UPoseAsset*> PoseDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionData> PoseMotionDatabase;

    // Current Motion State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_PoseCandidate BestPoseCandidate;

    // Motion Matching Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    // Character References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    class ACharacter* OwnerCharacter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    class UAnimInstance* AnimInstance;

public:
    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_PoseCandidate FindBestPoseMatch();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculatePoseMatchCost(const FAnim_MotionData& TargetMotion, const FAnim_MotionData& PoseMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ApplyPoseMatch(const FAnim_PoseCandidate& PoseCandidate);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildPoseDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddPoseToDatabase(class UPoseAsset* PoseAsset, const FAnim_MotionData& MotionData);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FVector GetCharacterVelocity() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FVector GetCharacterAcceleration() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetCharacterSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetCharacterDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsCharacterMoving() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsCharacterJumping() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsCharacterFalling() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsCharacterCrouching() const;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_PoseCandidate GetBestPoseCandidate() const { return BestPoseCandidate; }

private:
    FVector PreviousVelocity;
    float DeltaTimeAccumulator;
    bool bIsInitialized;

    void InitializeComponent();
    void CacheCharacterReferences();
    void ValidateDatabase();
};