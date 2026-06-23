#include "DinoSurvivorAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    FootIKTraceDistance = 55.0f;
    FootIKInterpSpeed   = 15.0f;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter    = Cast<ACharacter>(GetOwningActor());
    MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        OwnerCharacter    = Cast<ACharacter>(GetOwningActor());
        MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
        return;
    }

    // ── Core locomotion values ────────────────────────────────────────────────
    const FVector Velocity = MovementComponent->Velocity;
    Speed     = Velocity.Size2D();
    bIsInAir  = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Direction (signed angle between actor forward and velocity)
    if (Speed > 1.0f)
    {
        const FRotator ActorRot  = OwnerCharacter->GetActorRotation();
        const FRotator VelRot    = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // Lean — lateral acceleration mapped to [-1,1]
    const FVector LocalAccel = OwnerCharacter->GetActorTransform().InverseTransformVector(
        MovementComponent->GetCurrentAcceleration());
    LeanAmount = FMath::Clamp(LocalAccel.Y / MovementComponent->GetMaxAcceleration(), -1.0f, 1.0f);

    // Attack cooldown
    if (bIsAttacking)
    {
        AttackCooldownTimer -= DeltaSeconds;
        if (AttackCooldownTimer <= 0.0f)
        {
            bIsAttacking = false;
        }
    }

    UpdateLocomotionState();
    UpdateAimOffset();
    UpdateSurvivalBlends(DeltaSeconds);
}

void UDinoSurvivorAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // Foot IK is computed on the game thread (requires world traces)
    // Heavy blend-tree logic can be moved here for MT safety in future
}

// ── Private helpers ───────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotionState()
{
    if (bIsInAir)
    {
        const float VerticalVel = MovementComponent ? MovementComponent->Velocity.Z : 0.0f;
        LocomotionState = (VerticalVel > 0.0f)
            ? EAnim_LocomotionState::Jump
            : EAnim_LocomotionState::Fall;
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = (Speed > 10.0f)
            ? EAnim_LocomotionState::CrouchWalk
            : EAnim_LocomotionState::Crouch;
        return;
    }

    // Walk speed threshold ~200, run ~400, sprint ~600
    if (Speed < 10.0f)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (Speed < 250.0f)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else if (Speed < 450.0f)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir)
    {
        // Smoothly disable IK when airborne
        FootIKData.LeftFootAlpha  = FMath::FInterpTo(FootIKData.LeftFootAlpha,  0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        return;
    }

    FVector  LeftLoc,  RightLoc;
    FRotator LeftRot,  RightRot;

    const bool bLeftHit  = TraceFootIK(FName("foot_l"), LeftLoc,  LeftRot);
    const bool bRightHit = TraceFootIK(FName("foot_r"), RightLoc, RightRot);

    // Interp foot locations
    FootIKData.LeftFootLocation  = FMath::VInterpTo(FootIKData.LeftFootLocation,  LeftLoc,  DeltaSeconds, FootIKInterpSpeed);
    FootIKData.RightFootLocation = FMath::VInterpTo(FootIKData.RightFootLocation, RightLoc, DeltaSeconds, FootIKInterpSpeed);
    FootIKData.LeftFootRotation  = FMath::RInterpTo(FootIKData.LeftFootRotation,  LeftRot,  DeltaSeconds, FootIKInterpSpeed);
    FootIKData.RightFootRotation = FMath::RInterpTo(FootIKData.RightFootRotation, RightRot, DeltaSeconds, FootIKInterpSpeed);

    FootIKData.LeftFootAlpha  = FMath::FInterpTo(FootIKData.LeftFootAlpha,  bLeftHit  ? 1.0f : 0.0f, DeltaSeconds, FootIKInterpSpeed);
    FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, bRightHit ? 1.0f : 0.0f, DeltaSeconds, FootIKInterpSpeed);

    // Hip offset — lower hips when one foot is on lower ground
    const float LeftZ  = FootIKData.LeftFootLocation.Z;
    const float RightZ = FootIKData.RightFootLocation.Z;
    const float MinZ   = FMath::Min(LeftZ, RightZ);
    const float CharZ  = OwnerCharacter->GetActorLocation().Z;
    FootIKData.HipOffset = FMath::FInterpTo(FootIKData.HipOffset, FMath::Min(0.0f, MinZ - CharZ), DeltaSeconds, FootIKInterpSpeed);
}

bool UDinoSurvivorAnimInstance::TraceFootIK(FName SocketName, FVector& OutLocation, FRotator& OutRotation)
{
    if (!OwnerCharacter) return false;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return false;

    const FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
    const FVector TraceStart = FVector(SocketLoc.X, SocketLoc.Y, SocketLoc.Z + FootIKTraceDistance);
    const FVector TraceEnd   = FVector(SocketLoc.X, SocketLoc.Y, SocketLoc.Z - FootIKTraceDistance);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
    if (bHit)
    {
        OutLocation = Hit.ImpactPoint;
        // Align foot to surface normal
        const FVector Forward = OwnerCharacter->GetActorForwardVector();
        OutRotation = UKismetMathLibrary::MakeRotFromZX(Hit.ImpactNormal, Forward);
    }
    return bHit;
}

void UDinoSurvivorAnimInstance::UpdateSurvivalBlends(float DeltaSeconds)
{
    // Exhaustion — driven by stamina
    const float TargetExhaustion = SurvivalState.bIsExhausted ? 1.0f : 0.0f;
    ExhaustionBlendWeight = FMath::FInterpTo(ExhaustionBlendWeight, TargetExhaustion, DeltaSeconds, 2.0f);

    // Injury — driven by health
    const float TargetInjury = SurvivalState.bIsInjured ? 1.0f : 0.0f;
    InjuryBlendWeight = FMath::FInterpTo(InjuryBlendWeight, TargetInjury, DeltaSeconds, 3.0f);

    // Fear — drives hunched/tense posture additive
    FearBlendWeight = FMath::FInterpTo(FearBlendWeight, SurvivalState.FearLevel, DeltaSeconds, 4.0f);

    // Auto-detect exhaustion from stamina
    if (SurvivalState.StaminaNormalized < 0.15f)
    {
        SurvivalState.bIsExhausted = true;
    }
    else if (SurvivalState.StaminaNormalized > 0.35f)
    {
        SurvivalState.bIsExhausted = false;
    }

    // Auto-detect injury from health
    if (SurvivalState.HealthNormalized < 0.3f)
    {
        SurvivalState.bIsInjured = true;
    }
    else if (SurvivalState.HealthNormalized > 0.5f)
    {
        SurvivalState.bIsInjured = false;
    }
}

void UDinoSurvivorAnimInstance::UpdateAimOffset()
{
    if (!OwnerCharacter) return;

    const AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    const FRotator ControlRot = Controller->GetControlRotation();
    const FRotator ActorRot   = OwnerCharacter->GetActorRotation();
    const FRotator Delta      = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::Clamp(Delta.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::Clamp(Delta.Yaw,   -90.0f, 90.0f);
}

// ── Blueprint-callable ────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::SetCombatStance(EAnim_CombatStance NewStance)
{
    CombatStance = NewStance;
}

void UDinoSurvivorAnimInstance::TriggerAttack()
{
    if (!bIsAttacking)
    {
        bIsAttacking = true;
        AttackCooldownTimer = AttackCooldownDuration;
    }
}

float UDinoSurvivorAnimInstance::GetSpeedNormalized() const
{
    if (!MovementComponent) return 0.0f;
    const float MaxSpeed = MovementComponent->MaxWalkSpeed;
    return MaxSpeed > 0.0f ? FMath::Clamp(Speed / MaxSpeed, 0.0f, 1.0f) : 0.0f;
}

bool UDinoSurvivorAnimInstance::IsMovingOnGround() const
{
    return !bIsInAir && Speed > 1.0f;
}
