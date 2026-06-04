#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    ESurvivalAction CurrentAction;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentAction = ESurvivalAction::None;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingPose
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    float TimeInAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    FAnim_MotionData MotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
    float MatchingScore;

    FAnim_MotionMatchingPose()
    {
        AnimSequence = nullptr;
        TimeInAnimation = 0.0f;
        MatchingScore = 0.0f;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Core
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionMatchingPose FindBestMatchingPose(const FAnim_MotionData& CurrentMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(const FVector& NewVelocity, bool bInAir, bool bCrouching, ESurvivalAction Action);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddPoseToDatabase(UAnimSequence* Animation, float TimeStamp, const FAnim_MotionData& MotionData);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionDatabase();

    // Blending and Transitions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void BlendToNewPose(const FAnim_MotionMatchingPose& NewPose, float BlendTime = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlaySurvivalMontage(ESurvivalAction Action);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsMotionMatchingActive() const { return bMotionMatchingEnabled; }

protected:
    // Motion Matching Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionMatchingPose> PoseDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<UAnimSequence*> IdleAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<UAnimSequence*> WalkAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<UAnimSequence*> RunAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<UAnimSequence*> CrouchAnimations;

    // Survival Action Montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Montages")
    TMap<ESurvivalAction, UAnimMontage*> SurvivalMontages;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FAnim_MotionMatchingPose CurrentPose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bMotionMatchingEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MotionMatchingUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PoseMatchingThreshold;

    // Internal
    float LastMotionUpdateTime;
    class USkeletalMeshComponent* OwnerMeshComponent;
    class UAnimInstance* OwnerAnimInstance;

private:
    float CalculatePoseMatchingScore(const FAnim_MotionData& Current, const FAnim_MotionData& Candidate);
    void InitializeSurvivalMontages();
    UAnimSequence* GetRandomAnimationFromArray(const TArray<UAnimSequence*>& AnimArray);
};