#include "DinoAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UDinoAnimInstance::UDinoAnimInstance()
{
    LocomotionState = EAnim_DinoLocomotionState::Idle;
    Speed = 0.0f;
    Direction = 0.0f;
    bIsAttacking = false;
    bIsDead = false;
    bIsRoaring = false;
    WalkSpeed = 200.0f;
    RunSpeed = 600.0f;
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

    // Get velocity-based speed
    FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size2D();

    // Calculate movement direction relative to actor forward
    FRotator ActorRotation = OwnerPawn->GetActorRotation();
    FVector LocalVelocity = ActorRotation.UnrotateVector(Velocity);
    Direction = UKismetMathLibrary::DegAtan2(LocalVelocity.Y, LocalVelocity.X);

    // Determine locomotion state
    if (bIsDead)
    {
        LocomotionState = EAnim_DinoLocomotionState::Death;
    }
    else if (bIsAttacking)
    {
        LocomotionState = EAnim_DinoLocomotionState::Attack;
    }
    else if (bIsRoaring)
    {
        LocomotionState = EAnim_DinoLocomotionState::Roar;
    }
    else if (Speed > RunSpeed * 0.5f)
    {
        LocomotionState = EAnim_DinoLocomotionState::Run;
    }
    else if (Speed > 10.0f)
    {
        LocomotionState = EAnim_DinoLocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_DinoLocomotionState::Idle;
    }
}
