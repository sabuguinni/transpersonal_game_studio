#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "../SharedTypes.h"
#include "PrimitiveAnimationController.generated.h"

// Animation state for primitive character movement
UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing")
};

// Animation blend data for smooth transitions
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float AimPitch = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float AimYaw = 0.0f;
};

/**
 * Animation controller for primitive human character
 * Handles basic locomotion and survival animations
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPrimitiveAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPrimitiveAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current animation state
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentState = EAnim_MovementState::Idle;

    // Animation blend parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    FAnim_BlendData BlendData;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* IdleMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* LocomotionBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* JumpMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* ClimbMontage = nullptr;

    // Animation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Parameters")
    float WalkSpeedThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Parameters")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Parameters")
    float BlendSpeed = 5.0f;

    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState(float Speed, bool bIsInAir, bool bIsCrouching);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayClimbAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAimOffset(float Pitch, float Yaw);

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_BlendData GetBlendData() const { return BlendData; }

private:
    // Internal state management
    void UpdateBlendData(float DeltaTime);
    void DetermineMovementState(float Speed, bool bIsInAir, bool bIsCrouching);
    
    // Animation playback
    void PlayMontageIfValid(class UAnimMontage* Montage);
    
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComp = nullptr;

    // Smooth blending values
    float TargetSpeed = 0.0f;
    float TargetDirection = 0.0f;
    float TargetAimPitch = 0.0f;
    float TargetAimYaw = 0.0f;
};