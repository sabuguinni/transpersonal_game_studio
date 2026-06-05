#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float GroundDistance;

    FAnim_MotionMatchingData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Acceleration = FVector::ZeroVector;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        GroundDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PositionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxCandidates;

    FAnim_MotionMatchingSettings()
    {
        VelocityWeight = 1.0f;
        AccelerationWeight = 0.5f;
        DirectionWeight = 0.8f;
        PositionWeight = 0.3f;
        SearchRadius = 100.0f;
        MaxCandidates = 10;
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

    // Motion Matching Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionMatchingData CurrentMotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionMatchingSettings MotionMatchingSettings;

    // Animation References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* CrouchMontage;

    // Character Reference
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionMatchingData& TargetData, const FAnim_MotionMatchingData& CandidateData);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetLocomotionBlendSpaceValues(float Speed, float Direction);

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsMoving() const { return CurrentMotionData.bIsMoving; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsInAir() const { return CurrentMotionData.bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsCrouching() const { return CurrentMotionData.bIsCrouching; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentSpeed() const { return CurrentMotionData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FVector GetCurrentVelocity() const { return CurrentMotionData.Velocity; }

    // Animation Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Motion Matching")
    void OnMotionStateChanged(EAnim_MovementState NewState, EAnim_MovementState PreviousState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Motion Matching")
    void OnJumpStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Motion Matching")
    void OnLanded();

private:
    // Internal state tracking
    EAnim_MovementState CurrentMovementState;
    EAnim_MovementState PreviousMovementState;
    
    float LastUpdateTime;
    FVector PreviousVelocity;
    FVector PreviousLocation;

    // Helper functions
    void UpdateMovementState();
    void UpdateVelocityAndAcceleration(float DeltaTime);
    void UpdateGroundDistance();
    EAnim_MovementState DetermineMovementState();
};