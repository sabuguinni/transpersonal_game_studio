#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float GroundDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float TurnRate;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Velocity = FVector::ZeroVector;
        Direction = FVector::ForwardVector;
        bIsInAir = false;
        bIsCrouching = false;
        GroundDistance = 0.0f;
        TurnRate = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
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

    // Motion matching data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    EAnim_MovementState CurrentMovementState;

    // Animation references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* LandingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* IdleAnimation;

    // Motion matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float WalkThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float RunThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "300.0", ClampMax = "800.0"))
    float SprintThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float StateChangeSmoothing;

    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MovementState DetermineMovementState();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayLandingAnimation();

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsMoving() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetNormalizedSpeed() const;

private:
    // Internal state
    class ACharacter* OwnerCharacter;
    class UCharacterMovementComponent* MovementComponent;
    
    // Smoothing variables
    float SmoothedSpeed;
    FVector SmoothedVelocity;
    
    // Helper functions
    void InitializeReferences();
    void CalculateGroundDistance();
    void SmoothMotionData(float DeltaTime);
};