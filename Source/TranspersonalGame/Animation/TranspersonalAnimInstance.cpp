// TranspersonalAnimInstance.cpp
// Animation Agent #10 — PROD_CYCLE_AUTO_20260628_007
// Player character AnimInstance implementation — full locomotion state machine

#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsAccelerating = false;
    LocomotionState = EAnim_LocomotionState::Idle;
    Stance = EAnim_StanceType::Upright;

    // Survival stat defaults
    HealthNormalized = 1.0f;
    StaminaNormalized = 1.0f;
    HungerNormalized = 1.0f;
    FearLevel = 0.0f;
    bIsExhausted = false;
    bIsInjured = false;
    bIsDead = false;

    // IK defaults
    bEnableFootIK = true;
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;
    PelvisOffset = FVector::ZeroVector;

    // Lean defaults
    LeanAngle = 0.0f;
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    // Cached refs
    OwnerCharacter = nullptr;
    OwnerMovement = nullptr;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        OwnerCharacter = Cast<ACharacter>(Pawn);
        if (OwnerCharacter)
        {
            OwnerMovement = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !OwnerMovement)
    {
        // Re-attempt cache on invalid refs
        APawn* Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            OwnerCharacter = Cast<ACharacter>(Pawn);
            if (OwnerCharacter)
            {
                OwnerMovement = OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    // ── Velocity & Speed ─────────────────────────────────────────────────────
    FVector Velocity = OwnerCharacter->GetVelocity();
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    Speed = HorizontalVelocity.Size();

    // ── Direction (relative to character facing) ──────────────────────────────
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    if (Speed > 1.0f)
    {
        FRotator VelocityRotation = HorizontalVelocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, ActorRotation).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // ── Movement Flags ────────────────────────────────────────────────────────
    bIsInAir = OwnerMovement->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsAccelerating = OwnerMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;

    // Sprint detection: speed > 400 and accelerating
    bIsSprinting = (Speed > 400.0f) && bIsAccelerating && !bIsInAir;

    // ── Lean (smooth) ─────────────────────────────────────────────────────────
    float TargetLean = FMath::Clamp(Direction * 0.3f, -25.0f, 25.0f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 6.0f);

    // ── Locomotion State Machine ──────────────────────────────────────────────
    UpdateLocomotionState();

    // ── Foot IK ───────────────────────────────────────────────────────────────
    if (bEnableFootIK && !bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        // Smoothly retract IK when in air
        LeftFootIKAlpha  = FMath::FInterpTo(LeftFootIKAlpha,  0.0f, DeltaSeconds, 8.0f);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, 8.0f);
        PelvisOffset     = FMath::VInterpTo(PelvisOffset, FVector::ZeroVector, DeltaSeconds, 8.0f);
    }
}

void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    // Dead overrides everything
    if (bIsDead)
    {
        LocomotionState = EAnim_LocomotionState::Dead;
        return;
    }

    // In air
    if (bIsInAir)
    {
        if (OwnerCharacter && OwnerCharacter->GetVelocity().Z > 50.0f)
        {
            LocomotionState = EAnim_LocomotionState::Jumping;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Falling;
        }
        return;
    }

    // Grounded states
    if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouching;
        return;
    }

    if (Speed < 10.0f)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprinting;
    }
    else if (Speed > 200.0f)
    {
        LocomotionState = EAnim_LocomotionState::Running;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Walking;
    }

    // Stance override based on survival stats
    if (bIsExhausted)
    {
        Stance = EAnim_StanceType::Exhausted;
    }
    else if (bIsInjured)
    {
        Stance = EAnim_StanceType::Combat; // Wounded combat stance
    }
    else
    {
        Stance = EAnim_StanceType::Upright;
    }
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Trace parameters
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerCharacter);
    TraceParams.bTraceComplex = false;

    const float TraceHalfHeight = 60.0f;
    const float FootHeight = 10.0f;
    const float IKInterpSpeed = 12.0f;

    // ── Left Foot ─────────────────────────────────────────────────────────────
    FVector LeftFootBoneLocation = OwnerCharacter->GetMesh()
        ? OwnerCharacter->GetMesh()->GetBoneLocation(FName("foot_l"))
        : OwnerCharacter->GetActorLocation();

    FVector LeftTraceStart = LeftFootBoneLocation + FVector(0.0f, 0.0f, TraceHalfHeight);
    FVector LeftTraceEnd   = LeftFootBoneLocation - FVector(0.0f, 0.0f, TraceHalfHeight);

    FHitResult LeftHit;
    bool bLeftHit = World->LineTraceSingleByChannel(
        LeftHit, LeftTraceStart, LeftTraceEnd,
        ECC_Visibility, TraceParams
    );

    FVector TargetLeftOffset = FVector::ZeroVector;
    float TargetLeftAlpha = 0.0f;
    if (bLeftHit)
    {
        float ZDiff = LeftHit.ImpactPoint.Z - OwnerCharacter->GetActorLocation().Z + FootHeight;
        TargetLeftOffset = FVector(0.0f, 0.0f, FMath::Clamp(ZDiff, -20.0f, 20.0f));
        TargetLeftAlpha = 1.0f;
    }

    LeftFootOffset  = FMath::VInterpTo(LeftFootOffset,  TargetLeftOffset,  DeltaSeconds, IKInterpSpeed);
    LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, TargetLeftAlpha,   DeltaSeconds, IKInterpSpeed);

    // ── Right Foot ────────────────────────────────────────────────────────────
    FVector RightFootBoneLocation = OwnerCharacter->GetMesh()
        ? OwnerCharacter->GetMesh()->GetBoneLocation(FName("foot_r"))
        : OwnerCharacter->GetActorLocation();

    FVector RightTraceStart = RightFootBoneLocation + FVector(0.0f, 0.0f, TraceHalfHeight);
    FVector RightTraceEnd   = RightFootBoneLocation - FVector(0.0f, 0.0f, TraceHalfHeight);

    FHitResult RightHit;
    bool bRightHit = World->LineTraceSingleByChannel(
        RightHit, RightTraceStart, RightTraceEnd,
        ECC_Visibility, TraceParams
    );

    FVector TargetRightOffset = FVector::ZeroVector;
    float TargetRightAlpha = 0.0f;
    if (bRightHit)
    {
        float ZDiff = RightHit.ImpactPoint.Z - OwnerCharacter->GetActorLocation().Z + FootHeight;
        TargetRightOffset = FVector(0.0f, 0.0f, FMath::Clamp(ZDiff, -20.0f, 20.0f));
        TargetRightAlpha = 1.0f;
    }

    RightFootOffset  = FMath::VInterpTo(RightFootOffset,  TargetRightOffset,  DeltaSeconds, IKInterpSpeed);
    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, TargetRightAlpha,   DeltaSeconds, IKInterpSpeed);

    // ── Pelvis Compensation ───────────────────────────────────────────────────
    // Lower pelvis by the largest downward foot offset to prevent foot clipping
    float MinZ = FMath::Min(LeftFootOffset.Z, RightFootOffset.Z);
    FVector TargetPelvis = FVector(0.0f, 0.0f, FMath::Min(0.0f, MinZ));
    PelvisOffset = FMath::VInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, IKInterpSpeed * 0.5f);
}

void UTranspersonalAnimInstance::SetSurvivalStats(
    float Health, float Stamina, float Hunger, float Fear)
{
    HealthNormalized  = FMath::Clamp(Health,  0.0f, 1.0f);
    StaminaNormalized = FMath::Clamp(Stamina, 0.0f, 1.0f);
    HungerNormalized  = FMath::Clamp(Hunger,  0.0f, 1.0f);
    FearLevel         = FMath::Clamp(Fear,    0.0f, 1.0f);

    bIsExhausted = (StaminaNormalized < 0.15f);
    bIsInjured   = (HealthNormalized  < 0.30f);
    bIsDead      = (HealthNormalized  <= 0.0f);
}

EAnim_LocomotionState UTranspersonalAnimInstance::GetLocomotionState() const
{
    return LocomotionState;
}

EAnim_StanceType UTranspersonalAnimInstance::GetStance() const
{
    return Stance;
}
