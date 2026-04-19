#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "SharedTypes.h"
#include "PrimitiveAnimationController.generated.h"

// Forward declarations
class USkeletalMeshComponent;
class UAnimInstance;

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
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling")
};

/**
 * Animation blend data for smooth transitions
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bLooping = true;

    FAnim_BlendData()
    {
        BlendTime = 0.2f;
        PlayRate = 1.0f;
        bLooping = true;
    }
};

/**
 * Primitive Animation Controller
 * Handles basic character animations for prehistoric survival gameplay
 * Focuses on realistic human movement without magical/spiritual elements
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPrimitiveAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPrimitiveAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE ANIMATION CONTROL ===
    
    /**
     * Set the current movement state and trigger appropriate animation
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    /**
     * Get the current movement state
     */
    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    /**
     * Update animation based on character velocity and input
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationFromMovement(const FVector& Velocity, bool bIsJumping, bool bIsCrouching);

    /**
     * Play a one-shot animation montage (for actions like drinking, crafting)
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayActionMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    /**
     * Stop any currently playing montage
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopCurrentMontage(float BlendOutTime = 0.25f);

    // === ANIMATION ASSETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> RunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> JumpStartAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> JumpLoopAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> JumpLandAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> CrouchIdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> CrouchWalkAnimation;

    // === BLEND SPACES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> MovementBlendSpace;

    // === ANIMATION SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    FAnim_BlendData IdleBlendData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    FAnim_BlendData WalkBlendData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    FAnim_BlendData RunBlendData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    FAnim_BlendData JumpBlendData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float JumpVelocityThreshold = 50.0f;

    // === DEBUGGING ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugAnimationStates = false;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogCurrentAnimationState();

private:
    // === INTERNAL STATE ===

    UPROPERTY()
    EAnim_MovementState CurrentMovementState = EAnim_MovementState::Idle;

    UPROPERTY()
    EAnim_MovementState PreviousMovementState = EAnim_MovementState::Idle;

    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY()
    TObjectPtr<UAnimInstance> AnimInstance;

    // Animation transition tracking
    float StateTransitionTimer = 0.0f;
    bool bIsTransitioning = false;

    // === INTERNAL METHODS ===

    /**
     * Find and cache the skeletal mesh component
     */
    void InitializeSkeletalMeshComponent();

    /**
     * Determine movement state from velocity
     */
    EAnim_MovementState CalculateMovementStateFromVelocity(const FVector& Velocity, bool bIsJumping, bool bIsCrouching);

    /**
     * Handle state transition logic
     */
    void HandleStateTransition(EAnim_MovementState NewState);

    /**
     * Apply animation for current state
     */
    void ApplyAnimationForState(EAnim_MovementState State);

    /**
     * Get blend data for a specific state
     */
    FAnim_BlendData GetBlendDataForState(EAnim_MovementState State);
};