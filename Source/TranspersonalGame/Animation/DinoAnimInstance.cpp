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

    FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size2D();
    bIsMoving = Speed > WalkSpeedThreshold;

    if (bIsMoving)
    {
        FRotator ActorRot = OwnerPawn->GetActorRotation();
        FRotator VelRot = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    UpdateLocomotionState();
}

void UDinoAnimInstance::UpdateLocomotionState()
{
    if (bIsDead)
    {
        LocomotionState = EAnim_DinoLocomotionState::Death;
        return;
    }
    if (bIsAttacking)
    {
        LocomotionState = EAnim_DinoLocomotionState::Attack;
        return;
    }
    if (Speed >= RunSpeedThreshold)
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

void UDinoAnimInstance::SetAttacking(bool bAttacking)
{
    bIsAttacking = bAttacking;
}

void UDinoAnimInstance::SetDead(bool bDead)
{
    bIsDead = bDead;
}
