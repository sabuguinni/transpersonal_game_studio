#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_MotionMatchingSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_MotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing")
};

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
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MotionState CurrentState;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_MotionState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseSearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TrajectoryWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 MaxSearchFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bEnableRootMotion;

    FAnim_MotionMatchingSettings()
    {
        PoseSearchRadius = 50.0f;
        TrajectoryWeight = 1.0f;
        VelocityWeight = 0.8f;
        MaxSearchFrames = 60;
        bEnableRootMotion = true;
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

    // Motion data tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionMatchingSettings MotionSettings;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> LandingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> IdleAnimation;

    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MotionState DetermineMotionState();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void TransitionToState(EAnim_MotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateDirectionAngle();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayMotionMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    // State queries
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsMoving() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsInAir() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FVector GetCurrentVelocity() const;

    // Setup functions
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void InitializeMotionMatching();

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetSkeletalMeshComponent(USkeletalMeshComponent* MeshComp);

private:
    // Internal state tracking
    EAnim_MotionState PreviousState;
    float StateTransitionTime;
    float TimeSinceLastTransition;

    // Motion history for prediction
    TArray<FAnim_MotionData> MotionHistory;
    static const int32 MaxMotionHistoryFrames = 30;

    // Internal functions
    void UpdateMotionHistory();
    void PredictFutureMotion(float PredictionTime, FAnim_MotionData& OutPredictedMotion);
    float CalculateMotionSimilarity(const FAnim_MotionData& A, const FAnim_MotionData& B);
    void SmoothTransition(EAnim_MotionState FromState, EAnim_MotionState ToState, float DeltaTime);
};