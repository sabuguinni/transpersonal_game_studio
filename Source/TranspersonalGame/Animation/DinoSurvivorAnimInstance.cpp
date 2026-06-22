// DinoSurvivorAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full AnimInstance implementation: locomotion state machine, bilateral foot IK,
// survival posture blending, aim offset, lean calculation.

#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsAiming = false;
    bIsInjured = false;
    bIsExhausted = false;

    // State defaults
    LocomotionState = EAnim_LocomotionState::Idle;
    SurvivalPosture = EAnim_SurvivalPosture::Upright;

    // IK defaults
    bEnableFootIK = true;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    PelvisAdjustmentSpeed = 8.0f;

    // Survival stat defaults
    HealthNormalized = 1.0f;
    StaminaNormalized = 1.0f;
    FearLevel = 0.0f;

    // Aim offset defaults
    AimOffsetData.AimYaw = 0.0f;
    AimOffsetData.AimPitch = 0.0f;
    AimOffsetData.AimAlpha = 0.0f;

    // Lean defaults
    LeanAngle = 0.0f;
    LeanInterpSpeed = 6.0f;

    // Internal
    PreviousVelocity = FVector::ZeroVector;
    AccelerationDirection = FVector::ZeroVector;
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
    UpdateSurvivalPosture(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateLean(DeltaSeconds);

    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaSeconds);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// LOCOMOTION
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();

    // Direction relative to actor forward
    if (Speed > 1.0f)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FVector LocalVel = ActorRot.UnrotateVector(Velocity);
        Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVel.Y, LocalVel.X));
    }
    else
    {
        Direction = 0.0f;
    }

    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Determine locomotion state
    EAnim_LocomotionState NewState = LocomotionState;

    if (bIsInAir)
    {
        if (Velocity.Z > 50.0f)
        {
            NewState = EAnim_LocomotionState::Jump;
        }
        else if (Velocity.Z < -50.0f)
        {
            NewState = EAnim_LocomotionState::Fall;
        }
    }
    else if (bIsCrouching)
    {
        NewState = (Speed > 10.0f) ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }
    else
    {
        if (Speed < 10.0f)
        {
            NewState = EAnim_LocomotionState::Idle;
        }
        else if (bIsSprinting && Speed > 400.0f)
        {
            NewState = EAnim_LocomotionState::Sprint;
        }
        else if (Speed > 200.0f)
        {
            NewState = EAnim_LocomotionState::Run;
        }
        else
        {
            NewState = EAnim_LocomotionState::Walk;
        }
    }

    // Landing transition: if we were falling and now grounded
    if ((LocomotionState == EAnim_LocomotionState::Fall || LocomotionState == EAnim_LocomotionState::Jump)
        && !bIsInAir)
    {
        NewState = EAnim_LocomotionState::Land;
    }

    LocomotionState = NewState;
    PreviousVelocity = Velocity;
}

// ─────────────────────────────────────────────────────────────────────────────
// SURVIVAL POSTURE
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalPosture(float DeltaSeconds)
{
    // Priority: Injured > Exhausted > Fleeing > Cautious > Upright
    if (bIsInjured && HealthNormalized < 0.3f)
    {
        SurvivalPosture = EAnim_SurvivalPosture::Injured;
    }
    else if (bIsExhausted || StaminaNormalized < 0.15f)
    {
        SurvivalPosture = EAnim_SurvivalPosture::Exhausted;
    }
    else if (FearLevel > 0.75f)
    {
        SurvivalPosture = EAnim_SurvivalPosture::Fleeing;
    }
    else if (FearLevel > 0.35f || bIsAiming)
    {
        SurvivalPosture = EAnim_SurvivalPosture::Cautious;
    }
    else
    {
        SurvivalPosture = EAnim_SurvivalPosture::Upright;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AIM OFFSET
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter)
    {
        return;
    }

    APawn* Pawn = Cast<APawn>(OwnerCharacter);
    if (!Pawn)
    {
        return;
    }

    AController* Controller = Pawn->GetController();
    if (!Controller)
    {
        AimOffsetData.AimYaw = 0.0f;
        AimOffsetData.AimPitch = 0.0f;
        AimOffsetData.AimAlpha = bIsAiming ? 1.0f : 0.0f;
        return;
    }

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();

    // Delta between control and actor rotation
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    // Clamp to aim offset range [-90, 90]
    AimOffsetData.AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.0f, 90.0f);
    AimOffsetData.AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    AimOffsetData.AimAlpha = bIsAiming ? 1.0f : 0.5f;  // 0.5 for idle look-around
}

// ─────────────────────────────────────────────────────────────────────────────
// LEAN
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    FVector Velocity = MovementComponent->Velocity;
    FVector Accel = MovementComponent->GetCurrentAcceleration();

    if (Speed < 10.0f)
    {
        LeanAngle = FMath::FInterpTo(LeanAngle, 0.0f, DeltaSeconds, LeanInterpSpeed);
        return;
    }

    // Project acceleration onto right vector for lateral lean
    FVector RightVec = OwnerCharacter->GetActorRightVector();
    float LateralAccel = FVector::DotProduct(Accel.GetSafeNormal(), RightVec);

    float TargetLean = LateralAccel * 15.0f;  // Max 15 degrees lean
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, LeanInterpSpeed);
}

// ─────────────────────────────────────────────────────────────────────────────
// FOOT IK
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter)
    {
        return;
    }

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        return;
    }

    // Trace for left foot
    FVector LeftFootBoneLocation = Mesh->GetSocketLocation(FName("foot_l"));
    FVector RightFootBoneLocation = Mesh->GetSocketLocation(FName("foot_r"));

    FAnim_FootIKData NewIKData;

    // Left foot trace
    FVector LeftTraceStart = LeftFootBoneLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
    FVector LeftTraceEnd = LeftFootBoneLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult LeftHit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bLeftHit = GetWorld()->LineTraceSingleByChannel(
        LeftHit, LeftTraceStart, LeftTraceEnd,
        ECollisionChannel::ECC_Visibility, QueryParams
    );

    if (bLeftHit)
    {
        NewIKData.LeftFootLocation = LeftHit.ImpactPoint;
        // Align foot to surface normal
        FRotator SurfaceRot = UKismetMathLibrary::MakeRotFromZX(LeftHit.ImpactNormal, OwnerCharacter->GetActorForwardVector());
        NewIKData.LeftFootRotation = FMath::RInterpTo(FootIKData.LeftFootRotation, SurfaceRot, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        NewIKData.LeftFootLocation = LeftFootBoneLocation;
        NewIKData.LeftFootRotation = FRotator::ZeroRotator;
    }

    // Right foot trace
    FVector RightTraceStart = RightFootBoneLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
    FVector RightTraceEnd = RightFootBoneLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult RightHit;
    bool bRightHit = GetWorld()->LineTraceSingleByChannel(
        RightHit, RightTraceStart, RightTraceEnd,
        ECollisionChannel::ECC_Visibility, QueryParams
    );

    if (bRightHit)
    {
        NewIKData.RightFootLocation = RightHit.ImpactPoint;
        FRotator SurfaceRot = UKismetMathLibrary::MakeRotFromZX(RightHit.ImpactNormal, OwnerCharacter->GetActorForwardVector());
        NewIKData.RightFootRotation = FMath::RInterpTo(FootIKData.RightFootRotation, SurfaceRot, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        NewIKData.RightFootLocation = RightFootBoneLocation;
        NewIKData.RightFootRotation = FRotator::ZeroRotator;
    }

    // Pelvis offset: lower pelvis to accommodate the lower foot
    float LeftDelta = bLeftHit ? (LeftHit.ImpactPoint.Z - LeftFootBoneLocation.Z) : 0.0f;
    float RightDelta = bRightHit ? (RightHit.ImpactPoint.Z - RightFootBoneLocation.Z) : 0.0f;
    float TargetPelvisOffset = FMath::Min(LeftDelta, RightDelta);

    NewIKData.PelvisOffset = FMath::FInterpTo(
        FootIKData.PelvisOffset,
        TargetPelvisOffset,
        DeltaSeconds,
        PelvisAdjustmentSpeed
    );

    // Interp foot locations
    NewIKData.LeftFootLocation = FMath::VInterpTo(
        FootIKData.LeftFootLocation, NewIKData.LeftFootLocation,
        DeltaSeconds, FootIKInterpSpeed
    );
    NewIKData.RightFootLocation = FMath::VInterpTo(
        FootIKData.RightFootLocation, NewIKData.RightFootLocation,
        DeltaSeconds, FootIKInterpSpeed
    );

    FootIKData = NewIKData;
}

// ─────────────────────────────────────────────────────────────────────────────
// PUBLIC SETTERS (called from Character or GameMode)
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::SetSurvivalStats(float Health, float Stamina, float Fear)
{
    HealthNormalized = FMath::Clamp(Health, 0.0f, 1.0f);
    StaminaNormalized = FMath::Clamp(Stamina, 0.0f, 1.0f);
    FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);

    bIsInjured = (HealthNormalized < 0.5f);
    bIsExhausted = (StaminaNormalized < 0.2f);
}

void UDinoSurvivorAnimInstance::SetSprintingState(bool bSprinting)
{
    bIsSprinting = bSprinting;
}

void UDinoSurvivorAnimInstance::SetAimingState(bool bAiming)
{
    bIsAiming = bAiming;
}

// ─────────────────────────────────────────────────────────────────────────────
// BLUEPRINT CALLABLE GETTERS
// ─────────────────────────────────────────────────────────────────────────────

EAnim_LocomotionState UDinoSurvivorAnimInstance::GetLocomotionState() const
{
    return LocomotionState;
}

EAnim_SurvivalPosture UDinoSurvivorAnimInstance::GetSurvivalPosture() const
{
    return SurvivalPosture;
}

FAnim_FootIKData UDinoSurvivorAnimInstance::GetFootIKData() const
{
    return FootIKData;
}

FAnim_AimOffsetData UDinoSurvivorAnimInstance::GetAimOffsetData() const
{
    return AimOffsetData;
}

float UDinoSurvivorAnimInstance::GetLeanAngle() const
{
    return LeanAngle;
}
