// DinoSurvivorAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Implements the AnimInstance for the prehistoric survivor character
// Handles locomotion state machine, foot IK, and survival posture blending

#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    // Locomotion defaults
    GroundSpeed = 0.0f;
    bIsMoving = false;
    bIsFalling = false;
    bIsCrouching = false;
    bIsSprinting = false;
    MovementDirection = 0.0f;
    LeanAmount = 0.0f;

    // Survival posture defaults
    CurrentSurvivalPosture = EAnim_SurvivalPosture::Upright;
    CurrentLocomotionState = EAnim_LocomotionState::Idle;

    // IK defaults
    FootIKData.LeftFootLocation = FVector::ZeroVector;
    FootIKData.RightFootLocation = FVector::ZeroVector;
    FootIKData.LeftFootRotation = FRotator::ZeroRotator;
    FootIKData.RightFootRotation = FRotator::ZeroRotator;
    FootIKData.PelvisOffset = 0.0f;
    FootIKData.bLeftFootGrounded = true;
    FootIKData.bRightFootGrounded = true;
    FootIKData.LeftFootAlpha = 1.0f;
    FootIKData.RightFootAlpha = 1.0f;

    // Aim offset defaults
    AimOffsetData.AimYaw = 0.0f;
    AimOffsetData.AimPitch = 0.0f;
    AimOffsetData.AimAlpha = 0.0f;
    AimOffsetData.bIsAiming = false;

    // Survival stats defaults
    StaminaRatio = 1.0f;
    FearLevel = 0.0f;
    InjuryLevel = 0.0f;

    // Foot IK trace params
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    PelvisInterpSpeed = 10.0f;

    // Transition speeds
    WalkSpeed = 200.0f;
    RunSpeed = 400.0f;
    SprintSpeed = 650.0f;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
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
    UpdateAimOffset(DeltaSeconds);
    UpdateSurvivalPosture();
}

void UDinoSurvivorAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    // Ground speed (horizontal only)
    FVector Velocity = MovementComponent->Velocity;
    GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();

    // Is moving threshold
    bIsMoving = GroundSpeed > 10.0f;

    // Falling state
    bIsFalling = MovementComponent->IsFalling();

    // Crouch state
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Sprint detection (above run threshold)
    bIsSprinting = GroundSpeed > RunSpeed && !bIsFalling;

    // Movement direction (for strafe blending)
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator VelocityRot = Velocity.Rotation();
        FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot);
        MovementDirection = DeltaRot.Yaw;
    }
    else
    {
        MovementDirection = FMath::FInterpTo(MovementDirection, 0.0f, DeltaSeconds, 5.0f);
    }

    // Lean amount (based on acceleration)
    FVector Accel = MovementComponent->GetCurrentAcceleration();
    float AccelSize = Accel.Size();
    float TargetLean = bIsMoving ? FMath::Clamp(AccelSize / MovementComponent->MaxAcceleration, -1.0f, 1.0f) : 0.0f;
    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, 8.0f);
}

void UDinoSurvivorAnimInstance::UpdateLocomotionState()
{
    EAnim_LocomotionState NewState = EAnim_LocomotionState::Idle;

    if (bIsFalling)
    {
        FVector Velocity = MovementComponent ? MovementComponent->Velocity : FVector::ZeroVector;
        NewState = (Velocity.Z > 0.0f) ? EAnim_LocomotionState::Jump : EAnim_LocomotionState::Fall;
    }
    else if (bIsCrouching)
    {
        NewState = bIsMoving ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }
    else if (bIsMoving)
    {
        if (bIsSprinting)
        {
            NewState = EAnim_LocomotionState::Sprint;
        }
        else if (GroundSpeed > WalkSpeed)
        {
            NewState = EAnim_LocomotionState::Run;
        }
        else
        {
            NewState = EAnim_LocomotionState::Walk;
        }
    }
    else
    {
        NewState = EAnim_LocomotionState::Idle;
    }

    CurrentLocomotionState = NewState;
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsFalling) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Trace for left foot
    FVector LeftFootSocket = OwnerCharacter->GetMesh() ?
        OwnerCharacter->GetMesh()->GetSocketLocation(FName("foot_l")) :
        OwnerCharacter->GetActorLocation();

    FVector RightFootSocket = OwnerCharacter->GetMesh() ?
        OwnerCharacter->GetMesh()->GetSocketLocation(FName("foot_r")) :
        OwnerCharacter->GetActorLocation();

    FHitResult LeftHit, RightHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    // Left foot trace
    FVector LeftStart = LeftFootSocket + FVector(0, 0, FootIKTraceDistance);
    FVector LeftEnd = LeftFootSocket - FVector(0, 0, FootIKTraceDistance);
    bool bLeftHit = World->LineTraceSingleByChannel(LeftHit, LeftStart, LeftEnd, ECC_Visibility, Params);

    // Right foot trace
    FVector RightStart = RightFootSocket + FVector(0, 0, FootIKTraceDistance);
    FVector RightEnd = RightFootSocket - FVector(0, 0, FootIKTraceDistance);
    bool bRightHit = World->LineTraceSingleByChannel(RightHit, RightStart, RightEnd, ECC_Visibility, Params);

    // Update foot IK data
    FootIKData.bLeftFootGrounded = bLeftHit;
    FootIKData.bRightFootGrounded = bRightHit;

    if (bLeftHit)
    {
        FVector TargetLeft = LeftHit.ImpactPoint;
        FootIKData.LeftFootLocation = FMath::VInterpTo(FootIKData.LeftFootLocation, TargetLeft, DeltaSeconds, FootIKInterpSpeed);
        FRotator LeftNormalRot = FRotator(FMath::RadiansToDegrees(FMath::Atan2(LeftHit.ImpactNormal.X, LeftHit.ImpactNormal.Z)), 0.0f, -FMath::RadiansToDegrees(FMath::Atan2(LeftHit.ImpactNormal.Y, LeftHit.ImpactNormal.Z)));
        FootIKData.LeftFootRotation = FMath::RInterpTo(FootIKData.LeftFootRotation, LeftNormalRot, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    if (bRightHit)
    {
        FVector TargetRight = RightHit.ImpactPoint;
        FootIKData.RightFootLocation = FMath::VInterpTo(FootIKData.RightFootLocation, TargetRight, DeltaSeconds, FootIKInterpSpeed);
        FRotator RightNormalRot = FRotator(FMath::RadiansToDegrees(FMath::Atan2(RightHit.ImpactNormal.X, RightHit.ImpactNormal.Z)), 0.0f, -FMath::RadiansToDegrees(FMath::Atan2(RightHit.ImpactNormal.Y, RightHit.ImpactNormal.Z)));
        FootIKData.RightFootRotation = FMath::RInterpTo(FootIKData.RightFootRotation, RightNormalRot, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    // Pelvis offset: lower pelvis to match lowest foot
    float LeftDelta = bLeftHit ? (LeftHit.ImpactPoint.Z - LeftFootSocket.Z) : 0.0f;
    float RightDelta = bRightHit ? (RightHit.ImpactPoint.Z - RightFootSocket.Z) : 0.0f;
    float TargetPelvis = FMath::Min(LeftDelta, RightDelta);
    FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, TargetPelvis, DeltaSeconds, PelvisInterpSpeed);
}

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Get aim direction relative to character
    FRotator CharRot = OwnerCharacter->GetActorRotation();
    FRotator ControlRot = OwnerCharacter->GetControlRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, CharRot);

    AimOffsetData.AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.0f, 90.0f);
    AimOffsetData.AimPitch = FMath::ClampAngle(ControlRot.Pitch, -90.0f, 90.0f);
}

void UDinoSurvivorAnimInstance::UpdateSurvivalPosture()
{
    // Determine survival posture based on fear and injury levels
    if (InjuryLevel > 0.7f)
    {
        CurrentSurvivalPosture = EAnim_SurvivalPosture::Injured;
    }
    else if (FearLevel > 0.8f)
    {
        CurrentSurvivalPosture = EAnim_SurvivalPosture::Fleeing;
    }
    else if (bIsCrouching || FearLevel > 0.4f)
    {
        CurrentSurvivalPosture = EAnim_SurvivalPosture::Cautious;
    }
    else if (StaminaRatio < 0.2f)
    {
        CurrentSurvivalPosture = EAnim_SurvivalPosture::Exhausted;
    }
    else
    {
        CurrentSurvivalPosture = EAnim_SurvivalPosture::Upright;
    }
}

void UDinoSurvivorAnimInstance::SetSurvivalStats(float Stamina, float Fear, float Injury)
{
    StaminaRatio = FMath::Clamp(Stamina, 0.0f, 1.0f);
    FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);
    InjuryLevel = FMath::Clamp(Injury, 0.0f, 1.0f);
}

void UDinoSurvivorAnimInstance::TriggerJumpAnimation()
{
    if (!bIsFalling)
    {
        CurrentLocomotionState = EAnim_LocomotionState::Jump;
    }
}

void UDinoSurvivorAnimInstance::TriggerLandAnimation()
{
    CurrentLocomotionState = EAnim_LocomotionState::Land;
}

EAnim_LocomotionState UDinoSurvivorAnimInstance::GetLocomotionState() const
{
    return CurrentLocomotionState;
}

float UDinoSurvivorAnimInstance::GetGroundSpeed() const
{
    return GroundSpeed;
}

bool UDinoSurvivorAnimInstance::GetIsMoving() const
{
    return bIsMoving;
}

FAnim_FootIKData UDinoSurvivorAnimInstance::GetFootIKData() const
{
    return FootIKData;
}
