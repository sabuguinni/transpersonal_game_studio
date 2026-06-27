#include "DinoAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UDinoAnimInstance::UDinoAnimInstance()
{
    WalkSpeedThreshold = 50.f;
    RunSpeedThreshold = 300.f;
}

void UDinoAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        ACharacter* OwnerChar = Cast<ACharacter>(OwnerPawn);
        if (OwnerChar)
        {
            MovementComp = OwnerChar->GetCharacterMovement();
        }
    }
}

void UDinoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn) return;

        ACharacter* OwnerChar = Cast<ACharacter>(OwnerPawn);
        if (OwnerChar)
        {
            MovementComp = OwnerChar->GetCharacterMovement();
        }
    }

    // Update ground speed
    FVector Velocity = OwnerPawn->GetVelocity();
    Velocity.Z = 0.f;
    GroundSpeed = Velocity.Size();

    // Update air state
    if (MovementComp)
    {
        bIsInAir = MovementComp->IsFalling();
    }

    // Update movement direction (relative to actor forward)
    if (GroundSpeed > 1.f)
    {
        FRotator ActorRot = OwnerPawn->GetActorRotation();
        FVector LocalVelocity = ActorRot.UnrotateVector(OwnerPawn->GetVelocity());
        MovementDirection = UKismetMathLibrary::DegAtan2(LocalVelocity.Y, LocalVelocity.X);
    }
    else
    {
        MovementDirection = 0.f;
    }

    // Determine locomotion state
    LocomotionState = DetermineLocomotionState();

    // Sync locomotion data struct for Blueprint access
    LocomotionData.Speed = GroundSpeed;
    LocomotionData.Direction = MovementDirection;
    LocomotionData.bIsInAir = bIsInAir;
    LocomotionData.bIsAttacking = bIsAttacking;
    LocomotionData.bIsRoaring = bIsRoaring;
    LocomotionData.bIsDead = bIsDead;
    LocomotionData.bIsEating = bIsEating;
    LocomotionData.LocomotionState = LocomotionState;
}

EAnim_DinoLocomotionState UDinoAnimInstance::DetermineLocomotionState() const
{
    // Priority order: Death > Attack > Roar > Eating > Locomotion
    if (bIsDead)
        return EAnim_DinoLocomotionState::Death;

    if (bIsAttacking)
        return EAnim_DinoLocomotionState::Attack;

    if (bIsRoaring)
        return EAnim_DinoLocomotionState::Roar;

    if (bIsEating)
        return EAnim_DinoLocomotionState::Eating;

    if (bIsInAir)
        return EAnim_DinoLocomotionState::Idle; // Dinos don't fly — stay idle in air

    if (GroundSpeed >= RunSpeedThreshold)
        return EAnim_DinoLocomotionState::Run;

    if (GroundSpeed >= WalkSpeedThreshold)
        return EAnim_DinoLocomotionState::Walk;

    return EAnim_DinoLocomotionState::Idle;
}

void UDinoAnimInstance::TriggerAttack()
{
    if (bIsDead) return;
    bIsAttacking = true;
    // Montage playback is handled in Blueprint ABP via state machine transition
    // Reset after montage completes via Blueprint notify
}

void UDinoAnimInstance::TriggerRoar()
{
    if (bIsDead || bIsAttacking) return;
    bIsRoaring = true;
}

void UDinoAnimInstance::TriggerDeath()
{
    bIsDead = true;
    bIsAttacking = false;
    bIsRoaring = false;
    bIsEating = false;
}

void UDinoAnimInstance::SetEating(bool bEating)
{
    if (bIsDead || bIsAttacking) return;
    bIsEating = bEating;
}
