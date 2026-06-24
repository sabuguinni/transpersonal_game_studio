#include "DinoAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UDinoAnimInstance::UDinoAnimInstance()
{
    Speed = 0.0f;
    Direction = 0.0f;
    bIsMoving = false;
    bIsAttacking = false;
    bIsDead = false;
    LocomotionState = EAnim_DinoLocomotionState::Idle;
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    OwnerPawn = nullptr;
}

void UDinoAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerPawn = TryGetPawnOwner();
}

void UDinoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn) return;
    }

    // Calculate speed from velocity
    FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size();
    bIsMoving = Speed > WalkSpeedThreshold;

    // Calculate direction relative to actor forward
    if (bIsMoving)
    {
        FRotator ActorRotation = OwnerPawn->GetActorRotation();
        FVector VelocityNorm = Velocity.GetSafeNormal();
        Direction = UKismetMathLibrary::DegAtan2(
            FVector::DotProduct(VelocityNorm, ActorRotation.RotateVector(FVector::RightVector)),
            FVector::DotProduct(VelocityNorm, ActorRotation.RotateVector(FVector::ForwardVector))
        );
    }
    else
    {
        Direction = 0.0f;
    }

    // Determine locomotion state
    if (bIsDead)
    {
        LocomotionState = EAnim_DinoLocomotionState::Death;
    }
    else if (bIsAttacking)
    {
        LocomotionState = EAnim_DinoLocomotionState::Attack;
    }
    else if (Speed >= RunSpeedThreshold)
    {
        LocomotionState = EAnim_DinoLocomotionState::Run;
    }
    else if (Speed >= WalkSpeedThreshold)
    {
        LocomotionState = EAnim_DinoLocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_DinoLocomotionState::Idle;
    }
}
