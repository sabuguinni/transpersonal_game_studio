#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Core/SharedTypes.h"
#include "Anim_CharacterMovementController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_MovementMode MovementMode;

    FAnim_MovementState()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementMode = EAnim_MovementMode::Walking;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterMovementController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterMovementController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Movement state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FAnim_MovementState CurrentMovementState;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Idle")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Movement")
    class UBlendSpace* WalkRunBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Jump")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Crouch")
    class UAnimSequence* CrouchIdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Crouch")
    class UBlendSpace* CrouchWalkBlendSpace;

    // Movement thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Settings")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Settings")
    float DirectionSmoothingSpeed = 10.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_MovementMode GetCurrentMovementMode() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsMoving() const;

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Internal state
    float PreviousDirection;
    float DirectionSmoothingTimer;

    void CacheReferences();
    void UpdateAnimationBlueprint();
    EAnim_MovementMode DetermineMovementMode();
};