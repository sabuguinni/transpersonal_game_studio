#include "Anim_MotionMatchingController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingController::UAnim_MotionMatchingController()
{
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    WalkThreshold = 150.0f;
    RunThreshold = 400.0f;
    BlendSpeed = 5.0f;
}

void UAnim_MotionMatchingController::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_MotionMatchingController::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    UpdateMotionState(DeltaTimeX);
    UpdateBlendWeights(DeltaTimeX);
}

void UAnim_MotionMatchingController::UpdateMotionState(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update speed
    FVector Velocity = MovementComponent->Velocity;
    CurrentMotionState.Speed = Velocity.Size2D();

    // Update direction
    CurrentMotionState.Direction = CalculateDirection();

    // Update air state
    CurrentMotionState.bIsInAir = MovementComponent->IsFalling();

    // Update crouching state
    CurrentMotionState.bIsCrouching = MovementComponent->IsCrouching();

    // Update movement mode
    CurrentMotionState.MovementMode = DetermineMovementMode();
}

void UAnim_MotionMatchingController::UpdateBlendWeights(float DeltaTime)
{
    float TargetIdleWeight = 0.0f;
    float TargetWalkWeight = 0.0f;
    float TargetRunWeight = 0.0f;
    float TargetJumpWeight = 0.0f;

    if (CurrentMotionState.bIsInAir)
    {
        TargetJumpWeight = 1.0f;
    }
    else if (CurrentMotionState.Speed < WalkThreshold)
    {
        TargetIdleWeight = 1.0f;
    }
    else if (CurrentMotionState.Speed < RunThreshold)
    {
        TargetWalkWeight = 1.0f;
    }
    else
    {
        TargetRunWeight = 1.0f;
    }

    // Smooth blend transitions
    float InterpSpeed = BlendSpeed * DeltaTime;
    BlendWeights.IdleWeight = FMath::FInterpTo(BlendWeights.IdleWeight, TargetIdleWeight, DeltaTime, InterpSpeed);
    BlendWeights.WalkWeight = FMath::FInterpTo(BlendWeights.WalkWeight, TargetWalkWeight, DeltaTime, InterpSpeed);
    BlendWeights.RunWeight = FMath::FInterpTo(BlendWeights.RunWeight, TargetRunWeight, DeltaTime, InterpSpeed);
    BlendWeights.JumpWeight = FMath::FInterpTo(BlendWeights.JumpWeight, TargetJumpWeight, DeltaTime, InterpSpeed);
}

float UAnim_MotionMatchingController::CalculateDirection()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return 0.0f;
    }

    FVector Velocity = MovementComponent->Velocity;
    if (Velocity.Size2D() < 0.1f)
    {
        return 0.0f;
    }

    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector VelocityNormalized = Velocity.GetSafeNormal2D();

    float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
    float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;

    return FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
}

EAnim_MovementMode UAnim_MotionMatchingController::DetermineMovementMode()
{
    if (!MovementComponent)
    {
        return EAnim_MovementMode::Walking;
    }

    if (CurrentMotionState.bIsInAir)
    {
        return EAnim_MovementMode::Jumping;
    }

    if (CurrentMotionState.bIsCrouching)
    {
        return EAnim_MovementMode::Crouching;
    }

    if (CurrentMotionState.Speed >= RunThreshold)
    {
        return EAnim_MovementMode::Running;
    }

    if (CurrentMotionState.Speed >= WalkThreshold)
    {
        return EAnim_MovementMode::Walking;
    }

    return EAnim_MovementMode::Idle;
}