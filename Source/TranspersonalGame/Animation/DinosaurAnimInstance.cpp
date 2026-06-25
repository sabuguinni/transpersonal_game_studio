#include "DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    CurrentSpeed = 0.0f;
    CurrentState = EAnim_DinoLocomotionState::Idle;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn)
        {
            return;
        }
    }

    // Update speed from velocity
    FVector Velocity = OwnerPawn->GetVelocity();
    CurrentSpeed = Velocity.Size2D();

    // Update locomotion data
    LocomotionData.Speed = CurrentSpeed;

    // Determine locomotion state based on speed (if not in action state)
    if (!LocomotionData.bIsDead && !LocomotionData.bIsAttacking && !LocomotionData.bIsRoaring)
    {
        if (CurrentSpeed < WalkSpeedThreshold)
        {
            CurrentState = EAnim_DinoLocomotionState::Idle;
        }
        else if (CurrentSpeed < RunSpeedThreshold)
        {
            CurrentState = EAnim_DinoLocomotionState::Walk;
        }
        else
        {
            CurrentState = EAnim_DinoLocomotionState::Run;
        }
        LocomotionData.LocomotionState = CurrentState;
    }
}

void UDinosaurAnimInstance::TriggerAttack()
{
    if (LocomotionData.bIsDead)
    {
        return;
    }

    LocomotionData.bIsAttacking = true;
    CurrentState = EAnim_DinoLocomotionState::Attack;
    LocomotionData.LocomotionState = CurrentState;

    // Attack state resets after montage ends — handled by AnimNotify in Blueprint
}

void UDinosaurAnimInstance::TriggerRoar()
{
    if (LocomotionData.bIsDead || LocomotionData.bIsAttacking)
    {
        return;
    }

    LocomotionData.bIsRoaring = true;
    CurrentState = EAnim_DinoLocomotionState::Roar;
    LocomotionData.LocomotionState = CurrentState;
}

void UDinosaurAnimInstance::SetDead(bool bDead)
{
    LocomotionData.bIsDead = bDead;
    if (bDead)
    {
        CurrentState = EAnim_DinoLocomotionState::Death;
        LocomotionData.LocomotionState = CurrentState;
        LocomotionData.bIsAttacking = false;
        LocomotionData.bIsRoaring = false;
    }
}
