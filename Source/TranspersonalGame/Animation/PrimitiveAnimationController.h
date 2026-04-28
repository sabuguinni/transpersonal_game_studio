#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "PrimitiveAnimationController.generated.h"

// Forward declarations
class USkeletalMeshComponent;
class ACharacter;

/**
 * Animation states for primitive character movement
 */
UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching")
};

/**
 * Animation blend parameters for smooth transitions
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendParameters
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
    float LeanAmount = 0.0f;

    FAnim_BlendParameters()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        LeanAmount = 0.0f;
    }
};

/**
 * Primitive Animation Controller
 * Manages basic character animations for prehistoric survival gameplay
 * Handles movement state transitions and animation blending
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

    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_BlendParameters GetBlendParameters() const { return BlendParameters; }

    // Animation triggers
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerLandAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCrouchState(bool bShouldCrouch);

protected:
    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    // Animation state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState PreviousMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    FAnim_BlendParameters BlendParameters;

    // Animation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float AnimationBlendSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DirectionBlendSpeed = 8.0f;

private:
    // Internal state tracking
    float LastGroundedTime;
    bool bWasInAir;
    FVector LastVelocity;

    // Helper functions
    void InitializeComponents();
    void CalculateMovementState();
    void UpdateBlendParameters(float DeltaTime);
    void SmoothBlendValues(float DeltaTime);
    bool IsMoving() const;
    float CalculateMovementDirection() const;
};