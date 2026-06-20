#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(GetOwningActor());
        if (!OwnerCharacter) return;
    }

    UpdateLocomotion(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateSurvivalState(DeltaSeconds);

    // Foot IK runs on game thread (needs physics traces)
    UpdateFootIK(DeltaSeconds);
}

void UDinoSurvivorAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // Thread-safe updates (read-only data) can go here in future
}

// ── Locomotion ────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
    if (!MoveComp) return;

    const FVector Velocity = MoveComp->Velocity;
    const FVector HorizontalVel = FVector(Velocity.X, Velocity.Y, 0.0f);

    PreviousSpeed = Speed;
    Speed = HorizontalVel.Size();
    VerticalVelocity = Velocity.Z;

    bIsMoving = Speed > 3.0f;
    bIsInAir = MoveComp->IsFalling();
    bIsCrouching = MoveComp->IsCrouching();

    // Sprint: moving fast and not crouching
    bIsSprinting = Speed > 500.0f && !bIsCrouching && !bIsInAir;

    // Direction relative to actor forward
    if (bIsMoving)
    {
        const FRotator ActorRot = OwnerCharacter->GetActorRotation();
        const FRotator VelRot = HorizontalVel.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // Lean: rate of direction change
    const float SpeedDelta = Speed - PreviousSpeed;
    LeanAngle = FMath::FInterpTo(LeanAngle, SpeedDelta * 0.05f, DeltaSeconds, 5.0f);

    // Landed timer
    if (bJustLanded)
    {
        LandedTimer -= DeltaSeconds;
        if (LandedTimer <= 0.0f)
        {
            bJustLanded = false;
            LandedTimer = 0.0f;
        }
    }

    LocomotionState = DetermineLocomotionState();
}

EAnim_LocomotionState UDinoSurvivorAnimInstance::DetermineLocomotionState() const
{
    if (bJustLanded)  return EAnim_LocomotionState::Land;
    if (bIsInAir)     return EAnim_LocomotionState::InAir;
    if (bIsCrouching) return bIsMoving ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    if (bIsSprinting) return EAnim_LocomotionState::Sprint;
    if (Speed > 200.0f) return EAnim_LocomotionState::Run;
    if (bIsMoving)    return EAnim_LocomotionState::Walk;
    return EAnim_LocomotionState::Idle;
}

// ── Aim Offset ────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    const FRotator ActorRot = OwnerCharacter->GetActorRotation();
    const FRotator ControlRot = OwnerCharacter->GetControlRotation();
    const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::ClampAngle(Delta.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::ClampAngle(Delta.Yaw,   -90.0f, 90.0f);
}

// ── Foot IK ───────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    SolveFootIK(LeftFootIK,  FName("foot_l"), DeltaSeconds);
    SolveFootIK(RightFootIK, FName("foot_r"), DeltaSeconds);

    // Pelvis offset = lowest foot offset (so both feet stay on ground)
    const float LowestOffset = FMath::Min(LeftFootIK.HitDistance, RightFootIK.HitDistance);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, LowestOffset, DeltaSeconds, FootIKInterpSpeed);
}

void UDinoSurvivorAnimInstance::SolveFootIK(FAnim_FootIKData& FootData, FName SocketName, float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    UWorld* World = GetWorld();
    if (!World) return;

    const FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    const FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
    const FVector TraceEnd   = SocketLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    const bool bHit = World->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd,
        ECC_Visibility, QueryParams
    );

    if (bHit)
    {
        FootData.bFootOnGround = true;
        FootData.HitDistance = HitResult.Location.Z - SocketLocation.Z;

        // Smooth effector location
        const FVector TargetLocation = FVector(
            SocketLocation.X,
            SocketLocation.Y,
            HitResult.Location.Z
        );
        FootData.EffectorLocation = FMath::VInterpTo(
            FootData.EffectorLocation, TargetLocation, DeltaSeconds, FootIKInterpSpeed
        );

        // Foot rotation from surface normal
        const FVector Normal = HitResult.Normal;
        const FRotator TargetRot = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
            0.0f,
            -FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z))
        );
        FootData.FootRotation = FMath::RInterpTo(FootData.FootRotation, TargetRot, DeltaSeconds, FootIKInterpSpeed);
        FootData.Alpha = FMath::FInterpTo(FootData.Alpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        FootData.bFootOnGround = false;
        FootData.HitDistance = 0.0f;
        FootData.Alpha = FMath::FInterpTo(FootData.Alpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootData.FootRotation = FMath::RInterpTo(FootData.FootRotation, FRotator::ZeroRotator, DeltaSeconds, FootIKInterpSpeed);
    }
}

// ── Survival State ────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
    // These values are set externally by TranspersonalCharacter via SetSurvivalStats()
    // Here we derive boolean flags from the normalized values
    bIsExhausted = StaminaNormalized < 0.15f;
    bIsInjured   = HealthNormalized  < 0.30f;
    bIsFleeing   = FearNormalized    > 0.75f && bIsMoving;
}

// ── Blueprint Callable ────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::SetCombatStance(EAnim_CombatStance NewStance)
{
    CombatStance = NewStance;
}

void UDinoSurvivorAnimInstance::TriggerAttack()
{
    bIsAttacking = true;
    AttackTimer = 0.5f; // Will be cleared by montage end notify in Blueprint
}

float UDinoSurvivorAnimInstance::GetLocomotionBlendWeight() const
{
    // Returns 0..1 blend weight for locomotion state machine
    // Used by AnimBlueprint to blend between idle and movement
    return FMath::Clamp(Speed / 600.0f, 0.0f, 1.0f);
}
