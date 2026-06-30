// PlayerAnimInstance.cpp
// Animation Agent #10 — PROD_CYCLE_AUTO_20260630_001
// Prehistoric human survivor — locomotion, foot IK, survival overlays

#include "Animation/PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    FootTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    bEnableFootIK = true;
    WalkSpeedThreshold = 10.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 550.0f;
    SpeedSmoothingRate = 8.0f;
    LeanSmoothingRate = 5.0f;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
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

    // Raw velocity data
    const FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;

    // Smooth speed
    SmoothLocomotionValues(DeltaSeconds);

    // Direction relative to character facing
    Direction = GetMovementDirection();

    // State flags
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsMoving = Speed > WalkSpeedThreshold;

    // Land detection
    if (bWasInAir && !bIsInAir)
    {
        LandedTimer = 0.35f;
        OnLanded();
    }
    bWasInAir = bIsInAir;
    if (LandedTimer > 0.0f) LandedTimer -= DeltaSeconds;

    // Locomotion state machine
    UpdateLocomotionState();

    // Survival overlays (fear tremor, exhaustion hunch, wounded limp)
    UpdateSurvivalOverlays(DeltaSeconds);

    // Foot IK
    if (bEnableFootIK && !bIsInAir)
    {
        UpdateFootIK(DeltaSeconds);
    }
    else
    {
        // Reset IK when airborne
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    // Aim offset
    UpdateAimOffset(DeltaSeconds);
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    // Thread-safe pre-pass — only read cached values, no world queries
    // Lean smoothing is safe here
    const float TargetLean = bIsMoving ? FMath::Clamp(Direction / 90.0f, -1.0f, 1.0f) : 0.0f;
    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, LeanSmoothingRate);
}

// ─── Locomotion State Machine ─────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (bIsInAir)
    {
        LocomotionState = EAnim_LocomotionState::InAir;
        return;
    }

    if (LandedTimer > 0.0f)
    {
        LocomotionState = EAnim_LocomotionState::Land;
        return;
    }

    if (bIsClimbing)
    {
        LocomotionState = EAnim_LocomotionState::Climb;
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = bIsSneaking ? EAnim_LocomotionState::Sneak : EAnim_LocomotionState::Crouch;
        return;
    }

    if (!bIsMoving)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
        return;
    }

    if (SmoothedSpeed >= SprintSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (SmoothedSpeed >= RunSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
}

// ─── Survival Overlays ────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateSurvivalOverlays(float DeltaSeconds)
{
    FearLevel = SurvivalState.Fear;
    StaminaLevel = SurvivalState.Stamina;

    // Exhaustion alpha — ramps up when stamina < 0.2 and not sprinting
    const float ExhaustionTarget = (SurvivalState.bIsExhausted || StaminaLevel < 0.2f) ? 1.0f : 0.0f;
    ExhaustionAlpha = FMath::FInterpTo(ExhaustionAlpha, ExhaustionTarget, DeltaSeconds, 2.0f);

    // Wounded alpha — ramps up when health < 0.3
    const float WoundedTarget = (SurvivalState.bIsWounded || SurvivalState.Health < 0.3f) ? 1.0f : 0.0f;
    WoundedAlpha = FMath::FInterpTo(WoundedAlpha, WoundedTarget, DeltaSeconds, 1.5f);
}

// ─── Foot IK ─────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    FVector LeftHitLoc, RightHitLoc;
    FRotator LeftHitRot, RightHitRot;

    const bool bLeftHit = PerformFootTrace(FName("foot_l"), LeftHitLoc, LeftHitRot);
    const bool bRightHit = PerformFootTrace(FName("foot_r"), RightHitLoc, RightHitRot);

    // Smooth foot positions
    FootIKData.LeftFootLocation = FMath::VInterpTo(
        FootIKData.LeftFootLocation, bLeftHit ? LeftHitLoc : FootIKData.LeftFootLocation,
        DeltaSeconds, FootIKInterpSpeed);

    FootIKData.RightFootLocation = FMath::VInterpTo(
        FootIKData.RightFootLocation, bRightHit ? RightHitLoc : FootIKData.RightFootLocation,
        DeltaSeconds, FootIKInterpSpeed);

    FootIKData.LeftFootRotation = FMath::RInterpTo(
        FootIKData.LeftFootRotation, bLeftHit ? LeftHitRot : FRotator::ZeroRotator,
        DeltaSeconds, FootIKInterpSpeed);

    FootIKData.RightFootRotation = FMath::RInterpTo(
        FootIKData.RightFootRotation, bRightHit ? RightHitRot : FRotator::ZeroRotator,
        DeltaSeconds, FootIKInterpSpeed);

    // IK alpha — full when standing, zero when moving fast
    const float IKAlphaTarget = bIsMoving && SmoothedSpeed > RunSpeedThreshold ? 0.0f : 1.0f;
    FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, bLeftHit ? IKAlphaTarget : 0.0f, DeltaSeconds, FootIKInterpSpeed);
    FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, bRightHit ? IKAlphaTarget : 0.0f, DeltaSeconds, FootIKInterpSpeed);

    // Pelvis offset — lower pelvis to accommodate highest foot raise
    const float LeftDelta = FootIKData.LeftFootLocation.Z - OwnerCharacter->GetActorLocation().Z;
    const float RightDelta = FootIKData.RightFootLocation.Z - OwnerCharacter->GetActorLocation().Z;
    FootIKData.PelvisOffset = FMath::Min(LeftDelta, RightDelta);
    FootIKData.PelvisOffset = FMath::Clamp(FootIKData.PelvisOffset, -30.0f, 0.0f);
}

bool UPlayerAnimInstance::PerformFootTrace(const FName& SocketName, FVector& OutHitLocation, FRotator& OutHitNormal) const
{
    if (!OwnerCharacter) return false;

    const USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return false;

    const FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    const FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, FootTraceDistance);
    const FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, FootTraceDistance * 2.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (bHit)
    {
        OutHitLocation = HitResult.ImpactPoint;
        // Convert surface normal to foot rotation
        const FVector Normal = HitResult.ImpactNormal;
        OutHitNormal = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
            0.0f,
            FMath::RadiansToDegrees(FMath::Atan2(-Normal.Y, Normal.Z)));
    }

    return bHit;
}

// ─── Aim Offset ───────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    const FRotator ActorRot = OwnerCharacter->GetActorRotation();
    const FRotator ControlRot = OwnerCharacter->GetControlRotation();
    const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);
}

// ─── Smooth Locomotion Values ─────────────────────────────────────────────────

void UPlayerAnimInstance::SmoothLocomotionValues(float DeltaSeconds)
{
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, SpeedSmoothingRate);
}

// ─── Public API ───────────────────────────────────────────────────────────────

float UPlayerAnimInstance::GetMovementDirection() const
{
    if (!OwnerCharacter) return 0.0f;

    const FVector Velocity = MovementComponent ? MovementComponent->Velocity : FVector::ZeroVector;
    if (Velocity.IsNearlyZero()) return 0.0f;

    const FRotator ActorRot = OwnerCharacter->GetActorRotation();
    const FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
    return FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
}

void UPlayerAnimInstance::SetCombatStance(EAnim_CombatStance NewStance)
{
    CombatStance = NewStance;
}

void UPlayerAnimInstance::TriggerAttack(float PlayRate)
{
    AttackPlayRate = PlayRate;
    bIsAttacking = true;
    OnAttackStarted();
}
