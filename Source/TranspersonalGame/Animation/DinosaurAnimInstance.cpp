#include "DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    GroundSpeed = 0.0f;
    bIsMoving = false;
    bIsAttacking = false;
    bIsDead = false;
    bIsRoaring = false;
    LocomotionState = EAnim_DinoLocomotionState::Idle;
    FootIKAlpha = 1.0f;
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    OwnerPawn = nullptr;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerPawn = TryGetPawnOwner();
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        return;
    }

    // Update ground speed from velocity
    FVector Velocity = OwnerPawn->GetVelocity();
    Velocity.Z = 0.0f;
    GroundSpeed = Velocity.Size();

    bIsMoving = GroundSpeed > WalkSpeedThreshold;

    UpdateLocomotionState();
    UpdateFootIK();
}

void UDinosaurAnimInstance::UpdateLocomotionState()
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

    if (bIsRoaring)
    {
        LocomotionState = EAnim_DinoLocomotionState::Roar;
        return;
    }

    if (GroundSpeed >= RunSpeedThreshold)
    {
        LocomotionState = EAnim_DinoLocomotionState::Run;
    }
    else if (GroundSpeed >= WalkSpeedThreshold)
    {
        LocomotionState = EAnim_DinoLocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_DinoLocomotionState::Idle;
    }
}

void UDinosaurAnimInstance::UpdateFootIK()
{
    if (!OwnerPawn)
    {
        return;
    }

    // Basic foot IK — trace down from foot bone positions to find ground contact
    UWorld* World = OwnerPawn->GetWorld();
    if (!World)
    {
        return;
    }

    // Foot IK alpha: full when idle/walking, reduced when running
    if (LocomotionState == EAnim_DinoLocomotionState::Run)
    {
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, 0.0f, World->GetDeltaSeconds(), 5.0f);
    }
    else
    {
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, 1.0f, World->GetDeltaSeconds(), 5.0f);
    }

    // Trace for left foot
    FVector ActorLoc = OwnerPawn->GetActorLocation();
    FVector LeftFootStart = ActorLoc + FVector(-50.0f, 40.0f, 50.0f);
    FVector LeftFootEnd = LeftFootStart + FVector(0.0f, 0.0f, -200.0f);

    FHitResult LeftHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    if (World->LineTraceSingleByChannel(LeftHit, LeftFootStart, LeftFootEnd, ECC_WorldStatic, Params))
    {
        LeftFootIKOffset = FVector(0.0f, 0.0f, LeftHit.ImpactPoint.Z - ActorLoc.Z);
    }
    else
    {
        LeftFootIKOffset = FVector::ZeroVector;
    }

    // Trace for right foot
    FVector RightFootStart = ActorLoc + FVector(-50.0f, -40.0f, 50.0f);
    FVector RightFootEnd = RightFootStart + FVector(0.0f, 0.0f, -200.0f);

    FHitResult RightHit;
    if (World->LineTraceSingleByChannel(RightHit, RightFootStart, RightFootEnd, ECC_WorldStatic, Params))
    {
        RightFootIKOffset = FVector(0.0f, 0.0f, RightHit.ImpactPoint.Z - ActorLoc.Z);
    }
    else
    {
        RightFootIKOffset = FVector::ZeroVector;
    }
}
