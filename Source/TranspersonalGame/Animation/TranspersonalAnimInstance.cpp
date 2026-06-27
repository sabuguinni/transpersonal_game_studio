#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// ============================================================
// Constructor
// ============================================================

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    LocomotionState         = EAnim_LocomotionState::Idle;
    StanceType              = EAnim_StanceType::Standing;
    Speed                   = 0.0f;
    Direction               = 0.0f;
    LeanAngle               = 0.0f;
    LeanAngleTarget         = 0.0f;
    VerticalVelocity        = 0.0f;
    bIsInAir                = false;
    bIsCrouching            = false;
    bIsSprinting            = false;
    bIsAlive                = true;

    // Foot IK defaults
    LeftFootEffectorLocation  = FVector::ZeroVector;
    RightFootEffectorLocation = FVector::ZeroVector;
    LeftFootRotation          = FRotator::ZeroRotator;
    RightFootRotation         = FRotator::ZeroRotator;
    PelvisOffset              = 0.0f;
    bFootIKEnabled            = false;

    // Survival blend defaults
    WoundedBlendWeight    = 0.0f;
    ExhaustionBlendWeight = 0.0f;
    ColdBlendWeight       = 0.0f;

    // Speed thresholds
    WalkThreshold   = 10.0f;
    SprintThreshold = 400.0f;

    // Internal
    PreviousVelocity = FVector::ZeroVector;
    OwnerCharacter   = nullptr;
    MovementComponent = nullptr;
}

// ============================================================
// NativeInitializeAnimation
// ============================================================

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter    = Cast<ACharacter>(Pawn);
    MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
}

// ============================================================
// NativeUpdateAnimation — called every frame
// ============================================================

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent) return;

    UpdateLocomotion(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalBlends(DeltaSeconds);
}

// ============================================================
// UpdateLocomotion
// ============================================================

void UTranspersonalAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent) return;

    const FVector Velocity = OwnerCharacter->GetVelocity();
    Speed          = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;
    bIsInAir       = MovementComponent->IsFalling();
    bIsCrouching   = OwnerCharacter->bIsCrouched;
    bIsSprinting   = Speed > SprintThreshold;
    bIsAlive       = true; // Will be driven by health system when available

    // Direction: angle between actor forward and velocity vector
    if (Speed > WalkThreshold)
    {
        const FRotator ActorRot = OwnerCharacter->GetActorRotation();
        const FRotator VelRot   = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // Lean: based on lateral acceleration change
    const FVector CurrentVelocity = Velocity;
    const FVector DeltaVel        = (CurrentVelocity - PreviousVelocity) / FMath::Max(DeltaSeconds, 0.001f);
    const FVector LocalDeltaVel   = OwnerCharacter->GetActorTransform().InverseTransformVector(DeltaVel);
    LeanAngleTarget = FMath::Clamp(LocalDeltaVel.Y * 0.01f, -15.0f, 15.0f);
    LeanAngle       = FMath::FInterpTo(LeanAngle, LeanAngleTarget, DeltaSeconds, 8.0f);
    PreviousVelocity = CurrentVelocity;

    // Stance
    if (bIsCrouching)
    {
        StanceType = EAnim_StanceType::Crouched;
    }
    else
    {
        StanceType = EAnim_StanceType::Standing;
    }

    // Locomotion state machine
    LocomotionState = DetermineLocomotionState();

    // Foot IK only active when grounded and slow
    bFootIKEnabled = !bIsInAir && Speed < SprintThreshold;
}

// ============================================================
// DetermineLocomotionState
// ============================================================

EAnim_LocomotionState UTranspersonalAnimInstance::DetermineLocomotionState() const
{
    if (!bIsAlive)         return EAnim_LocomotionState::Dead;
    if (bIsInAir)
    {
        return VerticalVelocity > 0.0f
            ? EAnim_LocomotionState::Jumping
            : EAnim_LocomotionState::Falling;
    }
    if (bIsCrouching)      return EAnim_LocomotionState::Crouching;
    if (Speed > SprintThreshold) return EAnim_LocomotionState::Sprinting;
    if (Speed > WalkThreshold)   return EAnim_LocomotionState::Walking;
    return EAnim_LocomotionState::Idle;
}

// ============================================================
// UpdateFootIK
// ============================================================

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!bFootIKEnabled || !OwnerCharacter) return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    // Helper lambda: trace from foot bone location downward
    auto TraceFootIK = [&](const FName& BoneName, FVector& OutEffector, FRotator& OutRotation)
    {
        const USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
        if (!Mesh) return;

        const FVector BoneLocation = Mesh->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
        const FVector TraceStart   = BoneLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
        const FVector TraceEnd     = BoneLocation - FVector(0.0f, 0.0f, FootIKTraceDistance * 2.0f);

        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            // Interpolate effector toward hit location
            const FVector TargetEffector = HitResult.ImpactPoint;
            OutEffector = FMath::VInterpTo(OutEffector, TargetEffector, DeltaSeconds, FootIKInterpSpeed);

            // Compute foot rotation from surface normal
            const FVector Normal = HitResult.ImpactNormal;
            const FRotator TargetRot = FRotationMatrix::MakeFromZX(Normal, OwnerCharacter->GetActorForwardVector()).Rotator();
            OutRotation = FMath::RInterpTo(OutRotation, TargetRot, DeltaSeconds, FootIKInterpSpeed);
        }
    };

    TraceFootIK(FName("foot_l"), LeftFootEffectorLocation,  LeftFootRotation);
    TraceFootIK(FName("foot_r"), RightFootEffectorLocation, RightFootRotation);

    // Pelvis offset: lower pelvis to keep both feet planted
    const float LeftDelta  = LeftFootEffectorLocation.Z  - OwnerCharacter->GetActorLocation().Z;
    const float RightDelta = RightFootEffectorLocation.Z - OwnerCharacter->GetActorLocation().Z;
    const float TargetPelvis = FMath::Min(LeftDelta, RightDelta);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, PelvisInterpSpeed);
}

// ============================================================
// UpdateSurvivalBlends
// ============================================================

void UTranspersonalAnimInstance::UpdateSurvivalBlends(float DeltaSeconds)
{
    // These will be driven by the character's survival stats component
    // For now, interpolate toward 0 (healthy defaults)
    // When TranspersonalCharacter exposes health/stamina/temperature,
    // read them here and compute blend weights.

    // Example: WoundedBlendWeight = 1.0f - (HealthPercent)
    // ExhaustionBlendWeight = 1.0f - (StaminaPercent)
    // ColdBlendWeight = FMath::Clamp((BodyTemp - MinTemp) / TempRange, 0.0f, 1.0f)

    WoundedBlendWeight    = FMath::FInterpTo(WoundedBlendWeight,    0.0f, DeltaSeconds, 2.0f);
    ExhaustionBlendWeight = FMath::FInterpTo(ExhaustionBlendWeight, 0.0f, DeltaSeconds, 2.0f);
    ColdBlendWeight       = FMath::FInterpTo(ColdBlendWeight,       0.0f, DeltaSeconds, 2.0f);
}
