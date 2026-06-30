#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    Speed = 0.f;
    Direction = 0.f;
    LeanAngle = 0.f;
    bIsMoving = false;
    bIsSprinting = false;
    bIsCrouching = false;
    bIsInAir = false;
    bIsClimbing = false;
    FallSpeed = 0.f;
    JumpPeakAlpha = 0.f;

    LocomotionState = EAnim_LocomotionState::Idle;
    WeaponState = EAnim_WeaponState::Unarmed;

    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKAlpha = 0.f;
    RightFootIKAlpha = 0.f;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    PelvisOffset = 0.f;

    StaminaNormalized = 1.f;
    FearLevel = 0.f;
    bIsExhausted = false;
    bIsInjured = false;

    AimPitch = 0.f;
    AimYaw = 0.f;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;

    FootIKTraceDistance = 50.f;
    FootIKInterpSpeed = 12.f;
    LeanInterpSpeed = 5.f;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        OwnerCharacter = Cast<ACharacter>(Pawn);
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // ── Velocity & Speed ──────────────────────────────────────────────────
    FVector Velocity = MovementComponent->Velocity;
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.f);
    Speed = HorizontalVelocity.Size();
    FallSpeed = Velocity.Z;

    // ── Direction (relative to actor forward) ────────────────────────────
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    Direction = UKismetMathLibrary::NormalizedDeltaRotator(
        UKismetMathLibrary::MakeRotFromX(HorizontalVelocity),
        ActorRotation
    ).Yaw;

    // ── Boolean flags ─────────────────────────────────────────────────────
    bIsMoving = Speed > 3.f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Sprint detection: speed > 500 and not crouching
    bIsSprinting = Speed > 500.f && !bIsCrouching && !bIsInAir;

    // Jump peak alpha: 0 = rising, 1 = falling
    if (bIsInAir)
    {
        JumpPeakAlpha = FMath::Clamp(FMath::Abs(FallSpeed) / 800.f, 0.f, 1.f);
        if (FallSpeed > 0.f) JumpPeakAlpha = 0.f; // still rising
    }
    else
    {
        JumpPeakAlpha = 0.f;
    }

    // ── Lean ─────────────────────────────────────────────────────────────
    float TargetLean = bIsMoving ? FMath::Clamp(Direction * 0.3f, -15.f, 15.f) : 0.f;
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, LeanInterpSpeed);

    // ── Aim Offset ────────────────────────────────────────────────────────
    FRotator ControlRot = OwnerCharacter->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.f, 90.f);
    AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.f, 90.f);

    // ── Update subsystems ─────────────────────────────────────────────────
    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalAnimParams();
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (bIsClimbing)
    {
        LocomotionState = EAnim_LocomotionState::Climb;
        return;
    }
    if (bIsInAir)
    {
        LocomotionState = EAnim_LocomotionState::InAir;
        return;
    }
    if (bIsCrouching)
    {
        LocomotionState = bIsMoving
            ? EAnim_LocomotionState::Sneak
            : EAnim_LocomotionState::Crouch;
        return;
    }
    if (!bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
        return;
    }
    if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
        return;
    }
    if (Speed > 250.f)
    {
        LocomotionState = EAnim_LocomotionState::Run;
        return;
    }
    LocomotionState = EAnim_LocomotionState::Walk;
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Only apply foot IK when grounded
    if (bIsInAir)
    {
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.f, DeltaSeconds, FootIKInterpSpeed);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.f, DeltaSeconds, FootIKInterpSpeed);
        PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.f, DeltaSeconds, FootIKInterpSpeed);
        return;
    }

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    auto TraceFootIK = [&](FName BoneName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
    {
        FVector BoneLocation = Mesh->GetBoneLocation(BoneName);
        FVector TraceStart = BoneLocation + FVector(0.f, 0.f, FootIKTraceDistance);
        FVector TraceEnd = BoneLocation - FVector(0.f, 0.f, FootIKTraceDistance * 2.f);

        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(OwnerCharacter);

        bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            QueryParams
        );

        if (bHit)
        {
            FVector TargetLocation = HitResult.ImpactPoint;
            OutLocation = FMath::VInterpTo(OutLocation, TargetLocation, DeltaSeconds, FootIKInterpSpeed);

            // Foot rotation from surface normal
            FVector Normal = HitResult.ImpactNormal;
            FRotator SurfaceRotation = UKismetMathLibrary::MakeRotFromZX(Normal, OwnerCharacter->GetActorForwardVector());
            OutRotation = FMath::RInterpTo(OutRotation, SurfaceRotation, DeltaSeconds, FootIKInterpSpeed);
            OutAlpha = FMath::FInterpTo(OutAlpha, 1.f, DeltaSeconds, FootIKInterpSpeed);
        }
        else
        {
            OutAlpha = FMath::FInterpTo(OutAlpha, 0.f, DeltaSeconds, FootIKInterpSpeed);
        }
    };

    TraceFootIK(FName("foot_l"), LeftFootIKLocation, LeftFootIKRotation, LeftFootIKAlpha);
    TraceFootIK(FName("foot_r"), RightFootIKLocation, RightFootIKRotation, RightFootIKAlpha);

    // Pelvis offset: lower pelvis by the larger foot offset to avoid sliding
    float LeftOffset = LeftFootIKLocation.Z - Mesh->GetBoneLocation(FName("foot_l")).Z;
    float RightOffset = RightFootIKLocation.Z - Mesh->GetBoneLocation(FName("foot_r")).Z;
    float TargetPelvisOffset = FMath::Min(LeftOffset, RightOffset);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaSeconds, FootIKInterpSpeed * 0.5f);
}

void UPlayerAnimInstance::UpdateSurvivalAnimParams()
{
    // Exhaustion: slow, heavy movement when stamina low
    bIsExhausted = StaminaNormalized < 0.15f;

    // Injured: limping animation when health low (placeholder — connect to character stats)
    // bIsInjured is set externally by the character BP/C++ via direct property access

    // Fear affects movement speed multiplier (handled in blend space weights)
    // FearLevel is set externally
}
