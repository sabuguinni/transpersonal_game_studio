// TranspersonalAnimInstance.cpp
// Animation Agent #10 — Prehistoric Survival Game
// Implements locomotion state machine: Idle / Walk / Run / Jump / Crouch

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
    bIsRunning = false;
    bIsIdle = true;
    LeanAngle = 0.0f;
    PitchOffset = 0.0f;
    FootIK_LeftAlpha = 0.0f;
    FootIK_RightAlpha = 0.0f;
    WalkSpeed = 300.0f;
    RunSpeed = 600.0f;
    CurrentLocomotionState = EAnim_LocomotionState::Idle;
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

    UpdateLocomotionData(DeltaSeconds);
    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
    UpdateLean(DeltaSeconds);
}

void UTranspersonalAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    // Velocity-based speed
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();

    // Direction relative to actor facing
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = ActorRotation.UnrotateVector(Velocity);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    // Air / crouch / run states
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsRunning = Speed > WalkSpeed + 50.0f;
    bIsIdle = Speed < 10.0f && !bIsInAir;

    // Aim pitch offset for upper body lean
    if (AController* Ctrl = OwnerCharacter->GetController())
    {
        FRotator CtrlRot = Ctrl->GetControlRotation();
        FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(CtrlRot, ActorRotation);
        PitchOffset = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    }
}

void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    EAnim_LocomotionState NewState = EAnim_LocomotionState::Idle;

    if (bIsInAir)
    {
        NewState = EAnim_LocomotionState::Jump;
    }
    else if (bIsCrouching)
    {
        NewState = EAnim_LocomotionState::Crouch;
    }
    else if (Speed > RunSpeed - 50.0f)
    {
        NewState = EAnim_LocomotionState::Run;
    }
    else if (Speed > 10.0f)
    {
        NewState = EAnim_LocomotionState::Walk;
    }
    else
    {
        NewState = EAnim_LocomotionState::Idle;
    }

    CurrentLocomotionState = NewState;
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir)
    {
        // Fade out IK when airborne
        FootIK_LeftAlpha = FMath::FInterpTo(FootIK_LeftAlpha, 0.0f, DeltaSeconds, 10.0f);
        FootIK_RightAlpha = FMath::FInterpTo(FootIK_RightAlpha, 0.0f, DeltaSeconds, 10.0f);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    // Trace from foot bone positions to ground
    FVector ActorLoc = OwnerCharacter->GetActorLocation();
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    // Left foot trace
    FVector LeftFootStart = ActorLoc + FVector(-20.0f, -15.0f, 100.0f);
    FVector LeftFootEnd   = ActorLoc + FVector(-20.0f, -15.0f, -100.0f);
    FHitResult LeftHit;
    bool bLeftHit = World->LineTraceSingleByChannel(LeftHit, LeftFootStart, LeftFootEnd, ECC_Visibility, Params);
    FootIK_LeftAlpha = FMath::FInterpTo(FootIK_LeftAlpha, bLeftHit ? 1.0f : 0.0f, DeltaSeconds, 10.0f);

    // Right foot trace
    FVector RightFootStart = ActorLoc + FVector(-20.0f, 15.0f, 100.0f);
    FVector RightFootEnd   = ActorLoc + FVector(-20.0f, 15.0f, -100.0f);
    FHitResult RightHit;
    bool bRightHit = World->LineTraceSingleByChannel(RightHit, RightFootStart, RightFootEnd, ECC_Visibility, Params);
    FootIK_RightAlpha = FMath::FInterpTo(FootIK_RightAlpha, bRightHit ? 1.0f : 0.0f, DeltaSeconds, 10.0f);
}

void UTranspersonalAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!MovementComponent) return;

    // Lateral lean based on acceleration direction
    FVector Accel = MovementComponent->GetCurrentAcceleration();
    if (OwnerCharacter)
    {
        FVector LocalAccel = OwnerCharacter->GetActorRotation().UnrotateVector(Accel);
        float TargetLean = FMath::Clamp(LocalAccel.Y / 600.0f * 15.0f, -15.0f, 15.0f);
        LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 6.0f);
    }
}
