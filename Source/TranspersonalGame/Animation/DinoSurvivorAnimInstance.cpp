// DinoSurvivorAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full AnimInstance for the prehistoric survivor character
// Implements: locomotion state machine, foot IK, aim offset, survival posture blending

#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bShouldMove = false;
    bIsAccelerating = false;

    // Foot IK defaults
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;
    PelvisOffset = FVector::ZeroVector;
    FootIKTraceDistance = 55.0f;
    FootIKInterpSpeed = 15.0f;

    // Aim offset defaults
    AimYaw = 0.0f;
    AimPitch = 0.0f;
    AimYawDelta = 0.0f;
    bIsAiming = false;

    // Survival posture defaults
    CurrentPosture = EAnim_SurvivalPosture::Upright;
    PostureBlendAlpha = 0.0f;
    SurvivalHealthRatio = 1.0f;
    SurvivalStaminaRatio = 1.0f;
    bIsFleeing = false;

    // Lean defaults
    LeanAmount = 0.0f;
    LeanInterpSpeed = 8.0f;

    // Jump/land defaults
    bIsJumping = false;
    bIsLanding = false;
    JumpStartSpeed = 0.0f;
    TimeSinceLastLand = 0.0f;

    // Internal
    OwnerCharacter = nullptr;
    OwnerMovement = nullptr;
    PreviousSpeed = 0.0f;
    PreviousVelocityDirection = FVector::ZeroVector;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter = Cast<ACharacter>(Pawn);
    if (OwnerCharacter)
    {
        OwnerMovement = OwnerCharacter->GetCharacterMovement();
    }
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !OwnerMovement) return;

    UpdateLocomotionState(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateSurvivalPosture(DeltaSeconds);
    UpdateLean(DeltaSeconds);
}

// ============================================================
// LOCOMOTION STATE
// ============================================================

void UDinoSurvivorAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    if (!OwnerCharacter || !OwnerMovement) return;

    FVector Velocity = OwnerCharacter->GetVelocity();
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);

    Speed = HorizontalVelocity.Size();
    bIsInAir = OwnerMovement->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsAccelerating = OwnerMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;
    bShouldMove = Speed > 3.0f && bIsAccelerating;

    // Sprint detection: speed above walk threshold
    const float WalkMaxSpeed = OwnerMovement->MaxWalkSpeed;
    bIsSprinting = Speed > WalkMaxSpeed * 0.85f && !bIsInAir && !bIsCrouching;

    // Direction (angle between character forward and velocity)
    if (Speed > 1.0f)
    {
        FRotator CharRot = OwnerCharacter->GetActorRotation();
        FVector ForwardDir = UKismetMathLibrary::GetForwardVector(CharRot);
        FVector NormalizedVel = HorizontalVelocity.GetSafeNormal();
        Direction = UKismetMathLibrary::DegAtan2(
            FVector::DotProduct(NormalizedVel, UKismetMathLibrary::GetRightVector(CharRot)),
            FVector::DotProduct(NormalizedVel, ForwardDir)
        );
    }
    else
    {
        Direction = 0.0f;
    }

    // Jump/land state
    if (bIsInAir && !bIsJumping)
    {
        bIsJumping = true;
        bIsLanding = false;
        JumpStartSpeed = Speed;
    }
    else if (!bIsInAir && bIsJumping)
    {
        bIsJumping = false;
        bIsLanding = true;
        TimeSinceLastLand = 0.0f;
    }

    if (bIsLanding)
    {
        TimeSinceLastLand += DeltaSeconds;
        if (TimeSinceLastLand > 0.35f)
        {
            bIsLanding = false;
        }
    }

    PreviousSpeed = Speed;
    PreviousVelocityDirection = HorizontalVelocity.GetSafeNormal();
}

// ============================================================
// FOOT IK
// ============================================================

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir) 
    {
        // Smoothly reset IK when airborne
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        PelvisOffset = FMath::VInterpTo(PelvisOffset, FVector::ZeroVector, DeltaSeconds, FootIKInterpSpeed);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    // Trace for left foot
    FVector LeftFootTarget = TraceFootIK(World, TEXT("foot_l"), LeftFootOffset, DeltaSeconds);
    // Trace for right foot
    FVector RightFootTarget = TraceFootIK(World, TEXT("foot_r"), RightFootOffset, DeltaSeconds);

    // Pelvis correction: move pelvis down to accommodate the lower foot
    float LeftZ = LeftFootOffset.Z;
    float RightZ = RightFootOffset.Z;
    float TargetPelvisZ = FMath::Min(LeftZ, RightZ);
    PelvisOffset = FMath::VInterpTo(
        PelvisOffset,
        FVector(0.0f, 0.0f, TargetPelvisZ),
        DeltaSeconds,
        FootIKInterpSpeed * 0.5f
    );

    // IK alpha — fully active on ground
    float TargetAlpha = (Speed < 200.0f) ? 1.0f : FMath::Clamp(1.0f - (Speed - 200.0f) / 300.0f, 0.0f, 1.0f);
    LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, TargetAlpha, DeltaSeconds, FootIKInterpSpeed);
    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, TargetAlpha, DeltaSeconds, FootIKInterpSpeed);
}

FVector UDinoSurvivorAnimInstance::TraceFootIK(UWorld* World, const FName& FootBoneName, FVector& OutFootOffset, float DeltaSeconds)
{
    if (!OwnerCharacter || !World) return FVector::ZeroVector;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return FVector::ZeroVector;

    FVector BoneLocation = Mesh->GetBoneLocation(FootBoneName);
    FVector TraceStart = FVector(BoneLocation.X, BoneLocation.Y, BoneLocation.Z + FootIKTraceDistance);
    FVector TraceEnd   = FVector(BoneLocation.X, BoneLocation.Y, BoneLocation.Z - FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        Params
    );

    FVector TargetOffset = FVector::ZeroVector;
    if (bHit)
    {
        float HitZ = HitResult.Location.Z;
        float ActorZ = OwnerCharacter->GetActorLocation().Z;
        TargetOffset = FVector(0.0f, 0.0f, HitZ - ActorZ);
    }

    OutFootOffset = FMath::VInterpTo(OutFootOffset, TargetOffset, DeltaSeconds, FootIKInterpSpeed);
    return OutFootOffset;
}

// ============================================================
// AIM OFFSET
// ============================================================

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator ControlRot = OwnerCharacter->GetControlRotation();

    // Delta between control rotation and actor rotation
    FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimYaw   = FMath::ClampAngle(Delta.Yaw,   -90.0f, 90.0f);
    AimPitch = FMath::ClampAngle(Delta.Pitch, -90.0f, 90.0f);

    // Yaw delta for turn-in-place detection
    AimYawDelta = AimYaw - PreviousAimYaw;
    PreviousAimYaw = AimYaw;
}

// ============================================================
// SURVIVAL POSTURE
// ============================================================

void UDinoSurvivorAnimInstance::UpdateSurvivalPosture(float DeltaSeconds)
{
    EAnim_SurvivalPosture TargetPosture = EAnim_SurvivalPosture::Upright;

    if (bIsFleeing)
    {
        TargetPosture = EAnim_SurvivalPosture::Fleeing;
    }
    else if (SurvivalHealthRatio < 0.25f)
    {
        TargetPosture = EAnim_SurvivalPosture::Injured;
    }
    else if (SurvivalStaminaRatio < 0.2f)
    {
        TargetPosture = EAnim_SurvivalPosture::Exhausted;
    }
    else if (SurvivalHealthRatio < 0.5f || SurvivalStaminaRatio < 0.5f)
    {
        TargetPosture = EAnim_SurvivalPosture::Cautious;
    }

    // Blend alpha toward target
    float TargetAlpha = (TargetPosture != EAnim_SurvivalPosture::Upright) ? 1.0f : 0.0f;
    PostureBlendAlpha = FMath::FInterpTo(PostureBlendAlpha, TargetAlpha, DeltaSeconds, 3.0f);
    CurrentPosture = TargetPosture;
}

// ============================================================
// LEAN
// ============================================================

void UDinoSurvivorAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter || !OwnerMovement) return;

    FVector Velocity = OwnerCharacter->GetVelocity();
    FVector HorizVel = FVector(Velocity.X, Velocity.Y, 0.0f);

    if (HorizVel.SizeSquared() < 1.0f)
    {
        LeanAmount = FMath::FInterpTo(LeanAmount, 0.0f, DeltaSeconds, LeanInterpSpeed);
        return;
    }

    FVector Accel = OwnerMovement->GetCurrentAcceleration();
    FVector AccelDir = Accel.GetSafeNormal();
    FVector VelDir   = HorizVel.GetSafeNormal();

    FRotator CharRot = OwnerCharacter->GetActorRotation();
    FVector RightVec = UKismetMathLibrary::GetRightVector(CharRot);

    // Lean based on lateral acceleration
    float LateralAccel = FVector::DotProduct(AccelDir, RightVec) * Accel.Size();
    float TargetLean   = FMath::Clamp(LateralAccel / 600.0f, -1.0f, 1.0f);
    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, LeanInterpSpeed);
}

// ============================================================
// SURVIVAL STAT SETTERS (called from Character BP or C++)
// ============================================================

void UDinoSurvivorAnimInstance::SetSurvivalStats(float HealthRatio, float StaminaRatio, bool bFleeing)
{
    SurvivalHealthRatio  = FMath::Clamp(HealthRatio,  0.0f, 1.0f);
    SurvivalStaminaRatio = FMath::Clamp(StaminaRatio, 0.0f, 1.0f);
    bIsFleeing = bFleeing;
}

void UDinoSurvivorAnimInstance::SetAimingState(bool bAiming)
{
    bIsAiming = bAiming;
}
