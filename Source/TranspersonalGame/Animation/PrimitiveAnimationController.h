#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "PrimitiveAnimationController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Combat          UMETA(DisplayName = "Combat"),
    Interacting     UMETA(DisplayName = "Interacting"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundSpeed = 0.0f;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        Velocity = FVector::ZeroVector;
        GroundSpeed = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_StateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementState FromState = EAnim_MovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_MovementState ToState = EAnim_MovementState::Walking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TransitionDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bCanInterrupt = true;

    FAnim_StateTransition()
    {
        FromState = EAnim_MovementState::Idle;
        ToState = EAnim_MovementState::Walking;
        TransitionDuration = 0.2f;
        bCanInterrupt = true;
    }
};

/**
 * Primitive Animation Controller for basic character movement animations
 * Handles state transitions between idle, walk, run, jump, and other basic states
 * Designed to work with primitive survival characters in prehistoric environment
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPrimitiveAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPrimitiveAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetActionState(EAnim_ActionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_ActionState GetCurrentActionState() const { return CurrentActionState; }

    // Movement data access
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementSpeed() const { return MovementData.Speed; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsMoving() const { return MovementData.Speed > 5.0f; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsRunning() const { return MovementData.Speed > 300.0f; }

    // Action triggers
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerJump();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerLand();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerGatherAction();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerCombatAction();

    // Animation montage playback
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(UAnimMontage* Montage = nullptr);

    // State transition management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool CanTransitionTo(EAnim_MovementState NewState) const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void AddStateTransition(const FAnim_StateTransition& Transition);

protected:
    // Current animation states
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState PreviousMovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_ActionState CurrentActionState = EAnim_ActionState::None;

    // Movement data
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FAnim_MovementData MovementData;

    // State transition rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    TArray<FAnim_StateTransition> StateTransitions;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    float WalkSpeedThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    float IdleSpeedThreshold = 5.0f;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ACharacter* OwnerCharacter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* MovementComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    USkeletalMeshComponent* MeshComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UAnimInstance* AnimInstance = nullptr;

    // Internal state tracking
    float StateChangeTimer = 0.0f;
    bool bIsTransitioning = false;

private:
    void UpdateMovementData();
    void UpdateAnimationState();
    void InitializeStateTransitions();
    bool ValidateStateTransition(EAnim_MovementState FromState, EAnim_MovementState ToState) const;
};