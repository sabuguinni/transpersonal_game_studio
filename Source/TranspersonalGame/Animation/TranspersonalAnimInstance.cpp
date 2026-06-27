#include "TranspersonalAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    LocomotionState          = EAnim_LocomotionState::Idle;
    StanceType               = EAnim_StanceType::Standing;
    Speed                    = 0.0f;
    Direction                = 0.0f;
    LeanAngle                = 0.0f;
    VerticalVelocity         = 0.0f;
    bIsInAir                 = false;
    bIsCrouching             = false;
    bIsSprinting             = false;
    bIsMoving                = false;

    // Foot IK defaults
    LeftFootEffectorLocation  = FVector::ZeroVector;
    RightFootEffectorLocation = FVector::ZeroVector;
    LeftFootRotation          = FRotator::ZeroRotator;
    RightFootRotation         = FRotator::ZeroRotator;
    PelvisOffset              = 0.0f;
    bEnableFootIK             = true;

    // Survival defaults
    StaminaNormalized         = 1.0f;
    HealthNormalized          = 1.0f;
    bIsExhausted              = false;
    bIsInjured                = false;

    // Aim defaults
    AimPitch                  = 0.0f;
    AimYaw                    = 0.0f;

    // Blend space defaults
    BlendSpaceSpeed           = 0.0f;
    BlendSpaceDirection       = 0.0f;

    OwnerCharacter            = nullptr;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (!OwnerCharacter) return;
    }

    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalStats();
    UpdateAimOffsets();
}

// ─────────────────────────────────────────────────────────────────────────────
// Locomotion
// ─────────────────────────────────────────────────────────────────────────────
void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* MovComp =
        OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    const FVector Velocity  = OwnerCharacter->GetVelocity();
    const FVector HorizVel  = FVector(Velocity.X, Velocity.Y, 0.0f);

    Speed            = HorizVel.Size();
    VerticalVelocity = Velocity.Z;
    bIsInAir         = MovComp->IsFalling();
    bIsCrouching     = MovComp->IsCrouching();
    bIsMoving        = Speed > WalkSpeedThreshold;

    // Compute movement direction relative to actor rotation
    const FRotator ActorRot = OwnerCharacter->GetActorRotation();
    const FVector  LocalVel = ActorRot.UnrotateVector(HorizVel);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVel.Y, LocalVel.X));

    // Lean angle — lateral acceleration feel
    const FVector Accel = MovComp->GetCurrentAcceleration();
    const FVector LocalAccel = ActorRot.UnrotateVector(Accel);
    LeanAngle = FMath::Clamp(LocalAccel.Y * 0.02f, -15.0f, 15.0f);

    // Sprint detection: moving fast and not crouching
    bIsSprinting = bIsMoving && !bIsCrouching &&
                   Speed >= SprintSpeedThreshold;

    // Blend space mirrors
    BlendSpaceSpeed     = Speed;
    BlendSpaceDirection = Direction;

    // Stance
    if (bIsCrouching)
        StanceType = EAnim_StanceType::Crouched;
    else
        StanceType = EAnim_StanceType::Standing;

    // Locomotion state machine
    if (bIsInAir)
    {
        LocomotionState = (VerticalVelocity > 0.0f)
            ? EAnim_LocomotionState::Jumping
            : EAnim_LocomotionState::Falling;
    }
    else if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouching;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprinting;
    }
    else if (bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Walking;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Foot IK — two-bone IK trace per foot
// ─────────────────────────────────────────────────────────────────────────────
void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || !bEnableFootIK || bIsInAir) return;

    FVector  NewLeftLoc   = FVector::ZeroVector;
    FVector  NewRightLoc  = FVector::ZeroVector;
    FRotator NewLeftRot   = FRotator::ZeroRotator;
    FRotator NewRightRot  = FRotator::ZeroRotator;

    PerformFootTrace(TEXT("foot_l"), NewLeftLoc,  NewLeftRot);
    PerformFootTrace(TEXT("foot_r"), NewRightLoc, NewRightRot);

    // Smooth interpolation to avoid snapping
    LeftFootEffectorLocation = FMath::VInterpTo(
        LeftFootEffectorLocation, NewLeftLoc, DeltaSeconds, FootIKInterpSpeed);
    RightFootEffectorLocation = FMath::VInterpTo(
        RightFootEffectorLocation, NewRightLoc, DeltaSeconds, FootIKInterpSpeed);

    LeftFootRotation = FMath::RInterpTo(
        LeftFootRotation, NewLeftRot, DeltaSeconds, FootIKInterpSpeed);
    RightFootRotation = FMath::RInterpTo(
        RightFootRotation, NewRightRot, DeltaSeconds, FootIKInterpSpeed);

    // Pelvis offset = half the lowest foot drop so hips don't float
    const float LeftDrop  = LeftFootEffectorLocation.Z;
    const float RightDrop = RightFootEffectorLocation.Z;
    const float MinDrop   = FMath::Min(LeftDrop, RightDrop);
    PelvisOffset = FMath::FInterpTo(
        PelvisOffset, MinDrop * 0.5f, DeltaSeconds, FootIKInterpSpeed);
}

void UTranspersonalAnimInstance::PerformFootTrace(
    const FName& FootSocketName,
    FVector& OutEffectorLocation,
    FRotator& OutFootRotation)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    const FVector SocketLoc = Mesh->GetSocketLocation(FootSocketName);
    const FVector TraceStart = SocketLoc + FVector(0.0f, 0.0f, FootTraceLength);
    const FVector TraceEnd   = SocketLoc - FVector(0.0f, 0.0f, FootTraceLength * 2.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (bHit)
    {
        // Effector Z relative to capsule base
        const float CapsuleHalfHeight =
            OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        const float FootZ = HitResult.ImpactPoint.Z -
            (OwnerCharacter->GetActorLocation().Z - CapsuleHalfHeight);

        OutEffectorLocation = FVector(0.0f, 0.0f, FootZ);

        // Rotate foot to match surface normal
        const FVector SurfaceNormal = HitResult.ImpactNormal;
        const FRotator ActorRot     = OwnerCharacter->GetActorRotation();
        const FVector  ForwardVec   = ActorRot.Vector();
        const FVector  RightVec     = FVector::CrossProduct(SurfaceNormal, ForwardVec).GetSafeNormal();
        const FVector  AdjustedFwd  = FVector::CrossProduct(RightVec, SurfaceNormal).GetSafeNormal();

        OutFootRotation = UKismetMathLibrary::MakeRotFromXZ(AdjustedFwd, SurfaceNormal);
    }
    else
    {
        OutEffectorLocation = FVector::ZeroVector;
        OutFootRotation     = FRotator::ZeroRotator;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival Stats — read from character if it exposes them
// ─────────────────────────────────────────────────────────────────────────────
void UTranspersonalAnimInstance::UpdateSurvivalStats()
{
    // Default safe values — actual character stats hooked via Blueprint
    // or by casting to ATranspersonalCharacter in a subclass
    bIsExhausted = (StaminaNormalized < 0.1f);
    bIsInjured   = (HealthNormalized  < 0.3f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Aim Offsets
// ─────────────────────────────────────────────────────────────────────────────
void UTranspersonalAnimInstance::UpdateAimOffsets()
{
    if (!OwnerCharacter) return;

    const FRotator ControlRot = OwnerCharacter->GetControlRotation();
    const FRotator ActorRot   = OwnerCharacter->GetActorRotation();
    const FRotator DeltaRot   = UKismetMathLibrary::NormalizedDeltaRotator(
        ControlRot, ActorRot);

    AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::Clamp(DeltaRot.Yaw,   -90.0f, 90.0f);
}
