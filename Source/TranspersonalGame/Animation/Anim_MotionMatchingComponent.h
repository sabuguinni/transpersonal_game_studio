#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingComponent.generated.h"

// Motion Matching pose data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionPose
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector RootMotionVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector RootMotionDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FTransform> BoneTransforms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_MovementState MovementState;

    FAnim_MotionPose()
    {
        Timestamp = 0.0f;
        RootMotionVelocity = FVector::ZeroVector;
        RootMotionDirection = FVector::ForwardVector;
        MovementSpeed = 0.0f;
        MovementState = EAnim_MovementState::Idle;
    }
};

// Motion Matching database
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionDatabase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionPose> Poses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UAnimSequence* SourceAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FString DatabaseName;

    FAnim_MotionDatabase()
    {
        SourceAnimation = nullptr;
        DatabaseName = TEXT("Default");
    }
};

/**
 * Motion Matching Component for fluid character animation
 * Provides seamless transitions between movement states using pose matching
 */
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

    // Motion matching database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionDatabase> MotionDatabases;

    // Current pose matching settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseSearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BoneWeight;

    // Current motion state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionPose CurrentPose;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionPose TargetPose;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    float BlendAlpha;

    // Character reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    class ACharacter* OwnerCharacter;

    // Animation instance reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    class UAnimInstance* AnimInstance;

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatching(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionPose FindBestMatchingPose(const FAnim_MotionPose& QueryPose);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculatePoseDistance(const FAnim_MotionPose& PoseA, const FAnim_MotionPose& PoseB);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase(UAnimSequence* Animation, const FString& DatabaseName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    EAnim_MovementState GetCurrentMovementState() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToPose(const FAnim_MotionPose& NewPose, float BlendTime = 0.2f);

private:
    // Internal motion matching data
    float BlendTimer;
    float BlendDuration;
    bool bIsBlending;

    // Performance optimization
    float LastUpdateTime;
    float UpdateFrequency;

    // Helper functions
    void InitializeComponent();
    void UpdateCurrentPose();
    FVector GetCharacterVelocity() const;
    FVector GetCharacterDirection() const;
};