// TranspersonalAnimInstance.cpp
// Animation Agent #10 — PROD_CYCLE_AUTO_20260628_003
// Prehistoric survivor locomotion, foot IK, survival state animation

#include "TranspersonalAnimInstance.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    LocomotionState   = EAnim_LocomotionState::Idle;
    StanceType        = EAnim_StanceType::Standing;
    Speed             = 0.0f;
    Direction         = 0.0f;
    VerticalVelocity  = 0.0f;

    // State booleans
    bIsInAir          = false;
    bIsCrouching      = false;
    bIsSprinting      = false;
    bIsAttacking      = false;
    bIsDead           = false;
    bIsMoving         = false;

    // IK defaults
    LeftFootIKLocation  = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKAlpha     = 0.0f;
    RightFootIKAlpha    = 0.0f;
    PelvisOffset        = 0.0f;

    // Survival defaults
    FatigueLevel = 0.0f;
    FearLevel    = 0.0f;
    InjuryLevel  = 0.0f;

    // Aim offset defaults
    AimPitch = 0.0f;
    AimYaw   = 0.0f;

    // Internal
    OwnerCharacter    = nullptr;
    MovementComponent = nullptr;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        return;
    }

    OwnerCharacter = Cast<ATranspersonalCharacter>(OwnerPawn);
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
        // Re-attempt cache on first valid frame
        APawn* OwnerPawn = TryGetPawnOwner();
        if (OwnerPawn)
        {
            OwnerCharacter = Cast<ATranspersonalCharacter>(OwnerPawn);
            if (OwnerCharacter)
            {
                MovementComponent = OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    // ─── Velocity & Speed ────────────────────────────────────────────────────
    const FVector Velocity = MovementComponent->Velocity;
    const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);
    Speed = HorizontalVelocity.Size();
    VerticalVelocity = Velocity.Z;
    bIsMoving = Speed > MovingSpeedThreshold;

    // ─── Direction (strafe angle) ─────────────────────────────────────────────
    if (bIsMoving)
    {
        const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        const FRotator VelocityRotation = UKismetMathLibrary::MakeRotFromX(HorizontalVelocity);
        const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, ActorRotation);
        Direction = DeltaRot.Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // ─── Air State ───────────────────────────────────────────────────────────
    bIsInAir = MovementComponent->IsFalling();

    // ─── Crouch State ────────────────────────────────────────────────────────
    bIsCrouching = OwnerCharacter->bIsCrouched;
    StanceType = bIsCrouching ? EAnim_StanceType::Crouched : EAnim_StanceType::Standing;

    // ─── Sprint State ────────────────────────────────────────────────────────
    // Sprint = speed exceeds walk speed by 20%
    const float WalkSpeed = MovementComponent->MaxWalkSpeed;
    bIsSprinting = bIsMoving && !bIsInAir && (Speed > WalkSpeed * 0.85f);

    // ─── Locomotion State Machine ─────────────────────────────────────────────
    LocomotionState = DetermineLocomotionState();

    // ─── Aim Offset ──────────────────────────────────────────────────────────
    const FRotator BaseAimRotation = OwnerCharacter->GetBaseAimRotation();
    const FRotator ActorRot = OwnerCharacter->GetActorRotation();
    const FRotator AimDelta = UKismetMathLibrary::NormalizedDeltaRotator(BaseAimRotation, ActorRot);
    AimPitch = FMath::ClampAngle(AimDelta.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::ClampAngle(AimDelta.Yaw, -180.0f, 180.0f);

    // ─── Survival State ──────────────────────────────────────────────────────
    // Read from character survival stats if available
    // FatigueLevel, FearLevel, InjuryLevel are set externally by the character
    // They drive additive animation layers for posture/breathing/limping

    // ─── Foot IK ─────────────────────────────────────────────────────────────
    // Only run IK when grounded and not in air
    if (!bIsInAir)
    {
        UpdateFootIK(FName("foot_l"), LeftFootIKLocation, LeftFootIKAlpha);
        UpdateFootIK(FName("foot_r"), RightFootIKLocation, RightFootIKAlpha);

        // Pelvis offset: pull down by the larger of the two foot adjustments
        const float LeftDelta  = LeftFootIKLocation.Z  - OwnerCharacter->GetActorLocation().Z;
        const float RightDelta = RightFootIKLocation.Z - OwnerCharacter->GetActorLocation().Z;
        const float TargetPelvisOffset = FMath::Min(LeftDelta, RightDelta);
        PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaSeconds, IKInterpSpeed);
    }
    else
    {
        // In air: disable IK, reset pelvis
        LeftFootIKAlpha  = FMath::FInterpTo(LeftFootIKAlpha,  0.0f, DeltaSeconds, IKInterpSpeed);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, IKInterpSpeed);
        PelvisOffset     = FMath::FInterpTo(PelvisOffset,     0.0f, DeltaSeconds, IKInterpSpeed);
    }
}

EAnim_LocomotionState UTranspersonalAnimInstance::DetermineLocomotionState() const
{
    if (bIsDead)
    {
        return EAnim_LocomotionState::Dead;
    }
    if (bIsAttacking)
    {
        return EAnim_LocomotionState::Attacking;
    }
    if (bIsInAir)
    {
        return VerticalVelocity > 0.0f
            ? EAnim_LocomotionState::Jumping
            : EAnim_LocomotionState::Falling;
    }
    if (bIsCrouching)
    {
        return EAnim_LocomotionState::Crouching;
    }
    if (bIsSprinting)
    {
        return EAnim_LocomotionState::Sprinting;
    }
    if (bIsMoving)
    {
        return EAnim_LocomotionState::Walking;
    }
    return EAnim_LocomotionState::Idle;
}

void UTranspersonalAnimInstance::UpdateFootIK(
    const FName& FootSocketName,
    FVector& OutIKLocation,
    float& OutIKAlpha)
{
    if (!OwnerCharacter)
    {
        OutIKAlpha = 0.0f;
        return;
    }

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        OutIKAlpha = 0.0f;
        return;
    }

    // Get foot socket world location
    const FVector FootSocketLocation = Mesh->GetSocketLocation(FootSocketName);

    // Trace downward from foot
    const FVector TraceStart = FootSocketLocation + FVector(0.0f, 0.0f, FootIKTraceDistance * 0.5f);
    const FVector TraceEnd   = FootSocketLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
    {
        OutIKAlpha = 0.0f;
        return;
    }

    const bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Target IK location is the hit point
        const FVector TargetLocation = HitResult.ImpactPoint;
        OutIKLocation = FMath::VInterpTo(OutIKLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
        OutIKAlpha    = FMath::FInterpTo(OutIKAlpha, 1.0f, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
    }
    else
    {
        // No ground hit — disable IK for this foot
        OutIKAlpha = FMath::FInterpTo(OutIKAlpha, 0.0f, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
    }
}
