// TranspersonalAnimInstance.cpp
// Agent #10 — Animation Agent
// Implements the core Animation Instance for the prehistoric survivor character.
// Drives locomotion blend space, jump states, and foot IK.

#include "TranspersonalAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsIdle = true;
    bIsDead = false;
    bIsAttacking = false;
    bIsCarrying = false;
    LeanAngle = 0.0f;
    VerticalVelocity = 0.0f;
    FootIKLeftAlpha = 1.0f;
    FootIKRightAlpha = 1.0f;
    LeftFootEffectorLocation = FVector::ZeroVector;
    RightFootEffectorLocation = FVector::ZeroVector;
    StaminaRatio = 1.0f;
    FearLevel = 0.0f;
    LocomotionState = EAnim_LocomotionState::Idle;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        return;
    }

    UpdateLocomotionValues(DeltaSeconds);
    UpdateAirborneValues();
    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
}

void UTranspersonalAnimInstance::UpdateLocomotionValues(float DeltaSeconds)
{
    FVector Velocity = MovementComponent->Velocity;
    FVector VelocityXY = FVector(Velocity.X, Velocity.Y, 0.0f);
    Speed = VelocityXY.Size();

    VerticalVelocity = Velocity.Z;

    // Direction relative to character facing
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = ActorRotation.UnrotateVector(VelocityXY);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    // Lean based on acceleration
    FVector Acceleration = MovementComponent->GetCurrentAcceleration();
    FVector LocalAccel = ActorRotation.UnrotateVector(Acceleration);
    float TargetLean = FMath::Clamp(LocalAccel.Y * 0.05f, -15.0f, 15.0f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 5.0f);

    bIsIdle = Speed < 10.0f;
    bIsCrouching = MovementComponent->IsCrouching();
    bIsSprinting = Speed > 400.0f && !bIsInAir;
}

void UTranspersonalAnimInstance::UpdateAirborneValues()
{
    bIsInAir = MovementComponent->IsFalling();
}

void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    if (bIsDead)
    {
        LocomotionState = EAnim_LocomotionState::Dead;
        return;
    }
    if (bIsAttacking)
    {
        LocomotionState = EAnim_LocomotionState::Attacking;
        return;
    }
    if (bIsInAir)
    {
        LocomotionState = (VerticalVelocity > 0.0f)
            ? EAnim_LocomotionState::Jumping
            : EAnim_LocomotionState::Falling;
        return;
    }
    if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouching;
        return;
    }
    if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprinting;
        return;
    }
    if (Speed > 10.0f)
    {
        LocomotionState = EAnim_LocomotionState::Walking;
        return;
    }
    LocomotionState = EAnim_LocomotionState::Idle;
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir || bIsDead)
    {
        FootIKLeftAlpha = FMath::FInterpTo(FootIKLeftAlpha, 0.0f, DeltaSeconds, 10.0f);
        FootIKRightAlpha = FMath::FInterpTo(FootIKRightAlpha, 0.0f, DeltaSeconds, 10.0f);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    FVector ActorLocation = OwnerCharacter->GetActorLocation();
    float TraceHalfHeight = 50.0f;

    // Left foot IK trace
    FVector LeftFootSocket = OwnerCharacter->GetMesh()
        ? OwnerCharacter->GetMesh()->GetSocketLocation(FName("foot_l"))
        : ActorLocation + FVector(-20.0f, -15.0f, 0.0f);

    FHitResult LeftHit;
    FVector LeftStart = FVector(LeftFootSocket.X, LeftFootSocket.Y, ActorLocation.Z + TraceHalfHeight);
    FVector LeftEnd = FVector(LeftFootSocket.X, LeftFootSocket.Y, ActorLocation.Z - TraceHalfHeight);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bLeftHit = World->LineTraceSingleByChannel(LeftHit, LeftStart, LeftEnd, ECC_Visibility, QueryParams);
    if (bLeftHit)
    {
        LeftFootEffectorLocation = FVector(0.0f, 0.0f, LeftHit.Location.Z - ActorLocation.Z + 5.0f);
        FootIKLeftAlpha = FMath::FInterpTo(FootIKLeftAlpha, 1.0f, DeltaSeconds, 10.0f);
    }
    else
    {
        FootIKLeftAlpha = FMath::FInterpTo(FootIKLeftAlpha, 0.0f, DeltaSeconds, 10.0f);
    }

    // Right foot IK trace
    FVector RightFootSocket = OwnerCharacter->GetMesh()
        ? OwnerCharacter->GetMesh()->GetSocketLocation(FName("foot_r"))
        : ActorLocation + FVector(-20.0f, 15.0f, 0.0f);

    FHitResult RightHit;
    FVector RightStart = FVector(RightFootSocket.X, RightFootSocket.Y, ActorLocation.Z + TraceHalfHeight);
    FVector RightEnd = FVector(RightFootSocket.X, RightFootSocket.Y, ActorLocation.Z - TraceHalfHeight);

    bool bRightHit = World->LineTraceSingleByChannel(RightHit, RightStart, RightEnd, ECC_Visibility, QueryParams);
    if (bRightHit)
    {
        RightFootEffectorLocation = FVector(0.0f, 0.0f, RightHit.Location.Z - ActorLocation.Z + 5.0f);
        FootIKRightAlpha = FMath::FInterpTo(FootIKRightAlpha, 1.0f, DeltaSeconds, 10.0f);
    }
    else
    {
        FootIKRightAlpha = FMath::FInterpTo(FootIKRightAlpha, 0.0f, DeltaSeconds, 10.0f);
    }
}

void UTranspersonalAnimInstance::SetStaminaRatio(float InRatio)
{
    StaminaRatio = FMath::Clamp(InRatio, 0.0f, 1.0f);
}

void UTranspersonalAnimInstance::SetFearLevel(float InFear)
{
    FearLevel = FMath::Clamp(InFear, 0.0f, 1.0f);
}

void UTranspersonalAnimInstance::SetIsAttacking(bool bAttacking)
{
    bIsAttacking = bAttacking;
}

void UTranspersonalAnimInstance::SetIsCarrying(bool bCarry)
{
    bIsCarrying = bCarry;
}

void UTranspersonalAnimInstance::SetIsDead(bool bDead)
{
    bIsDead = bDead;
}
