#include "TranspersonalAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    FootTraceLength = 50.0f;
    FootIKInterpSpeed = 15.0f;
    PelvisInterpSpeed = 10.0f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
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
        OwnerCharacter = Cast<ACharacter>(GetOwningActor());
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        return;
    }

    // ─── Ground speed ────────────────────────────────────────────
    FVector Velocity = OwnerCharacter->GetVelocity();
    Velocity.Z = 0.0f;
    GroundSpeed = Velocity.Size();

    bIsMoving = GroundSpeed > 3.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsSprinting = GroundSpeed > 400.0f;

    // ─── Direction (strafe angle) ─────────────────────────────────
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator VelocityRot = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // ─── Lean (lateral acceleration) ─────────────────────────────
    FVector Accel = MovementComponent->GetCurrentAcceleration();
    float LateralAccel = FVector::DotProduct(Accel, OwnerCharacter->GetActorRightVector());
    LeanAngle = FMath::FInterpTo(LeanAngle, FMath::Clamp(LateralAccel * 0.02f, -15.0f, 15.0f), DeltaSeconds, 5.0f);

    // ─── Update subsystems ────────────────────────────────────────
    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalReactivity();

    // ─── Upper body layer weight (combat) ────────────────────────
    float TargetUpperBodyWeight = bIsAttacking ? 1.0f : 0.0f;
    UpperBodyLayerWeight = FMath::FInterpTo(UpperBodyLayerWeight, TargetUpperBodyWeight, DeltaSeconds, 8.0f);
}

void UTranspersonalAnimInstance::NativePostEvaluateAnimation()
{
    Super::NativePostEvaluateAnimation();
    // Post-evaluate: foot IK bone transforms applied here by AnimGraph
}

void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    if (bIsInAir)
    {
        if (OwnerCharacter && OwnerCharacter->GetVelocity().Z > 0.0f)
        {
            LocomotionState = EAnim_LocomotionState::Jump;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Fall;
        }
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouch;
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
    }
    else if (GroundSpeed > 200.0f)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir)
    {
        // Reset IK when in air
        LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, 0.0f, DeltaSeconds, 10.0f);
        RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, 0.0f, DeltaSeconds, 10.0f);
        PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.0f, DeltaSeconds, PelvisInterpSpeed);
        return;
    }

    SolveFootIK(FName("foot_l"), LeftFootIK, DeltaSeconds);
    SolveFootIK(FName("foot_r"), RightFootIK, DeltaSeconds);

    // Pelvis offset = lower of the two foot offsets to prevent stretching
    float TargetPelvisOffset = FMath::Min(LeftFootIK.FootLocation.Z, RightFootIK.FootLocation.Z);
    TargetPelvisOffset = FMath::Clamp(TargetPelvisOffset, -20.0f, 0.0f);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaSeconds, PelvisInterpSpeed);
}

void UTranspersonalAnimInstance::SolveFootIK(const FName& FootBoneName, FAnim_FootIKData& OutIKData, float DeltaSeconds)
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

    // Get bone world location
    FVector FootWorldLocation = Mesh->GetSocketLocation(FootBoneName);

    // Trace downward from foot to find ground
    FVector TraceStart = FootWorldLocation + FVector(0.0f, 0.0f, FootTraceLength);
    FVector TraceEnd   = FootWorldLocation - FVector(0.0f, 0.0f, FootTraceLength * 2.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    OutIKData.bFootOnGround = bHit;

    if (bHit)
    {
        // Calculate Z offset relative to character capsule base
        float CharacterBaseZ = OwnerCharacter->GetActorLocation().Z
            - OwnerCharacter->GetSimpleCollisionHalfHeight();
        float FootOffsetZ = HitResult.ImpactPoint.Z - CharacterBaseZ;

        // Smooth interpolation
        OutIKData.FootLocation = FMath::VInterpTo(
            OutIKData.FootLocation,
            FVector(0.0f, 0.0f, FootOffsetZ),
            DeltaSeconds,
            FootIKInterpSpeed
        );

        // Foot rotation from surface normal
        FRotator TargetRot = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(HitResult.ImpactNormal.X, HitResult.ImpactNormal.Z)),
            0.0f,
            -FMath::RadiansToDegrees(FMath::Atan2(HitResult.ImpactNormal.Y, HitResult.ImpactNormal.Z))
        );
        OutIKData.FootRotation = FMath::RInterpTo(OutIKData.FootRotation, TargetRot, DeltaSeconds, FootIKInterpSpeed);
        OutIKData.IKAlpha = FMath::FInterpTo(OutIKData.IKAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        OutIKData.FootLocation = FMath::VInterpTo(OutIKData.FootLocation, FVector::ZeroVector, DeltaSeconds, FootIKInterpSpeed);
        OutIKData.FootRotation = FMath::RInterpTo(OutIKData.FootRotation, FRotator::ZeroRotator, DeltaSeconds, FootIKInterpSpeed);
        OutIKData.IKAlpha = FMath::FInterpTo(OutIKData.IKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }
}

void UTranspersonalAnimInstance::UpdateSurvivalReactivity()
{
    // Exhaustion blend: kicks in when stamina < 30%
    float StaminaRatio = SurvivalState.Stamina / 100.0f;
    float TargetExhaustion = StaminaRatio < 0.3f ? (1.0f - (StaminaRatio / 0.3f)) : 0.0f;
    ExhaustionBlend = FMath::FInterpConstantTo(ExhaustionBlend, TargetExhaustion, GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f, 0.5f);

    // Wounded limping: kicks in when health < 40%
    float HealthRatio = SurvivalState.Health / 100.0f;
    float TargetLimping = HealthRatio < 0.4f ? (1.0f - (HealthRatio / 0.4f)) : 0.0f;
    WoundedLimping = FMath::FInterpConstantTo(WoundedLimping, TargetLimping, GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f, 0.3f);

    // Fear tremor: high fear causes subtle body tremor
    float TargetTremor = SurvivalState.Fear / 100.0f;
    FearTremor = FMath::FInterpTo(FearTremor, TargetTremor, GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f, 2.0f);
}

void UTranspersonalAnimInstance::SetLocomotionState(EAnim_LocomotionState NewState)
{
    LocomotionState = NewState;
}

void UTranspersonalAnimInstance::TriggerAttackMontage()
{
    bIsAttacking = true;
    // Montage play is handled by Blueprint or Character — AnimInstance signals readiness
    // Reset after a short delay (driven by montage notify in BP)
}

void UTranspersonalAnimInstance::TriggerJumpMontage()
{
    if (!bIsInAir)
    {
        LocomotionState = EAnim_LocomotionState::Jump;
    }
}

float UTranspersonalAnimInstance::GetSpeedNormalized() const
{
    // Returns 0.0 (idle) to 1.0 (full sprint at 600 units/s)
    return FMath::Clamp(GroundSpeed / 600.0f, 0.0f, 1.0f);
}
