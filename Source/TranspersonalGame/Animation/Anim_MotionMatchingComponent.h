#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TrajectoryRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 PoseSamples = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 TrajectorySamples = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius = 100.0f;

    FAnim_MotionMatchingSettings()
    {
        TrajectoryRadius = 100.0f;
        PoseSamples = 12;
        TrajectorySamples = 20;
        BlendTime = 0.15f;
        SearchRadius = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PoseSearchData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose Search")
    bool bIsGrounded;

    FAnim_PoseSearchData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsGrounded = true;
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

    // Motion Matching Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionMatchingSettings MotionMatchingSettings;

    // Current pose search data
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_PoseSearchData CurrentPoseData;

    // Animation montages for different actions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* InteractMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* HurtMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* JumpMontage;

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdatePoseSearchData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculatePoseDistance(const FAnim_PoseSearchData& PoseA, const FAnim_PoseSearchData& PoseB);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void StopAnimationMontage(UAnimMontage* Montage, float BlendOutTime = 0.25f);

    // Animation state queries
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsPlayingMontage(UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetMontagePosition(UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingEnabled(bool bEnabled);

    // Trajectory prediction
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    TArray<FVector> PredictTrajectory(float PredictionTime, int32 Samples);

    // Animation blending
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToAnimation(UAnimSequence* Animation, float BlendTime);

protected:
    // Internal state
    UPROPERTY()
    bool bMotionMatchingEnabled;

    UPROPERTY()
    USkeletalMeshComponent* OwnerMeshComponent;

    UPROPERTY()
    UAnimInstance* OwnerAnimInstance;

    // Trajectory history for motion matching
    UPROPERTY()
    TArray<FAnim_PoseSearchData> TrajectoryHistory;

    // Helper functions
    void InitializeComponent();
    void UpdateTrajectoryHistory();
    void CleanupTrajectoryHistory();
    
    // Animation event handlers
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};