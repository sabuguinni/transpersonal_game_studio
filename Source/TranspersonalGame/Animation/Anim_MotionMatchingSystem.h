#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingSystem.generated.h"

// Forward declarations
class UAnimSequence;
class UBlendSpace2D;

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
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_LocomotionType : uint8
{
    Ground      UMETA(DisplayName = "Ground Locomotion"),
    Air         UMETA(DisplayName = "Air Locomotion"),
    Water       UMETA(DisplayName = "Water Locomotion"),
    Climbing    UMETA(DisplayName = "Climbing Locomotion")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    EAnim_MovementState MovementState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    EAnim_LocomotionType LocomotionType;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
        LocomotionType = EAnim_LocomotionType::Ground;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    TSoftObjectPtr<UBlendSpace> IdleWalkRun;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    TSoftObjectPtr<UBlendSpace> CrouchMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    TSoftObjectPtr<UBlendSpace> AirMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    TSoftObjectPtr<UBlendSpace> ClimbingMovement;

    FAnim_BlendSpaceConfig()
    {
        IdleWalkRun = nullptr;
        CrouchMovement = nullptr;
        AirMovement = nullptr;
        ClimbingMovement = nullptr;
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

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MovementState DetermineMovementState() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetBlendSpaceConfiguration(const FAnim_BlendSpaceConfig& Config);

    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void TransitionToState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    bool CanTransitionToState(EAnim_MovementState NewState) const;

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void ResetAnimationState();

    // Getters for animation blueprint
    UFUNCTION(BlueprintPure, Category = "Motion Data")
    float GetSpeed() const { return CurrentMotionData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Motion Data")
    float GetDirection() const { return CurrentMotionData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Motion Data")
    bool IsMoving() const { return CurrentMotionData.bIsMoving; }

    UFUNCTION(BlueprintPure, Category = "Motion Data")
    bool IsInAir() const { return CurrentMotionData.bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Motion Data")
    bool IsCrouching() const { return CurrentMotionData.bIsCrouching; }

    UFUNCTION(BlueprintPure, Category = "Motion Data")
    EAnim_MovementState GetMovementState() const { return CurrentMotionData.MovementState; }

protected:
    // Character reference
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    // Movement component reference
    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    // Current motion data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    // Previous motion data for comparison
    UPROPERTY()
    FAnim_MotionData PreviousMotionData;

    // Blend space configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    FAnim_BlendSpaceConfig BlendSpaceConfig;

    // Movement thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float WalkThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float RunThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float SprintThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float MovingThreshold = 10.0f;

    // State transition timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Timing")
    float StateTransitionDelay = 0.1f;

    UPROPERTY()
    float LastStateChangeTime = 0.0f;

private:
    // Internal helper functions
    void InitializeReferences();
    void UpdateMovementState();
    void UpdateLocomotionType();
    bool IsValidForMotionMatching() const;
    float GetGroundSpeed() const;
    FVector GetMovementDirection() const;
};

#include "Anim_MotionMatchingSystem.generated.h"