// DinoSurvivorAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full AnimInstance implementation for the prehistoric survivor character

#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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
    LocomotionState = EAnim_LocomotionState::Idle;
    CombatStance = EAnim_CombatStance::Unarmed;

    // Survival stat defaults
    HealthNormalized = 1.0f;
    StaminaNormalized = 1.0f;
    FearLevel = 0.0f;
    HungerNormalized = 1.0f;

    // IK defaults
    FootIKLeftOffset = FVector::ZeroVector;
    FootIKRightOffset = FVector::ZeroVector;
    PelvisOffset = FVector::ZeroVector;
    bEnableFootIK = true;
    FootIKTraceDistance = 80.0f;
    FootIKInterpSpeed = 15.0f;
    PelvisInterpSpeed = 10.0f;

    // Aim defaults
    AimPitch = 0.0f;
    AimYaw = 0.0f;
    AimInterpSpeed = 8.0f;

    // Lean defaults
    LeanAngle = 0.0f;
    LeanInterpSpeed = 6.0f;

    // Upper body weight for survival blending
    UpperBodyBlendWeight = 0.0f;

    // Internal
    PreviousVelocity = FVector::ZeroVector;
    CurrentLeanAngle = 0.0f;
    CurrentAimPitch = 0.0f;
    CurrentAimYaw = 0.0f;
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
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        return;
    }

    UpdateLocomotionState(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateLean(DeltaSeconds);
    UpdateSurvivalBlend(DeltaSeconds);

    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaSeconds);
    }
}

// ─── Locomotion ───────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();

    // Direction relative to character facing
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Determine sprint: speed > 450 and not crouching
    bIsSprinting = (Speed > 450.0f && !bIsCrouching && !bIsInAir);

    // State machine
    if (bIsInAir)
    {
        if (Velocity.Z > 0.0f)
            LocomotionState = EAnim_LocomotionState::Jump;
        else
            LocomotionState = EAnim_LocomotionState::Fall;
    }
    else if (bIsCrouching)
    {
        LocomotionState = (Speed > 10.0f) ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }
    else if (Speed < 10.0f)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed > 200.0f)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
}

// ─── Aim Offset ───────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    AController* Controller = Pawn->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    float TargetPitch = FMath::ClampAngle(Delta.Pitch, -90.0f, 90.0f);
    float TargetYaw   = FMath::ClampAngle(Delta.Yaw,   -90.0f, 90.0f);

    CurrentAimPitch = FMath::FInterpTo(CurrentAimPitch, TargetPitch, DeltaSeconds, AimInterpSpeed);
    CurrentAimYaw   = FMath::FInterpTo(CurrentAimYaw,   TargetYaw,   DeltaSeconds, AimInterpSpeed);

    AimPitch = CurrentAimPitch;
    AimYaw   = CurrentAimYaw;
}

// ─── Lean ─────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLean(float DeltaSeconds)
{
    FVector CurrentVelocity = MovementComponent->Velocity;
    FVector VelocityDelta = (CurrentVelocity - PreviousVelocity) / FMath::Max(DeltaSeconds, 0.001f);
    PreviousVelocity = CurrentVelocity;

    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector LocalAccel = ActorRot.UnrotateVector(VelocityDelta);

    float TargetLean = FMath::Clamp(LocalAccel.Y * 0.02f, -45.0f, 45.0f);
    CurrentLeanAngle = FMath::FInterpTo(CurrentLeanAngle, TargetLean, DeltaSeconds, LeanInterpSpeed);
    LeanAngle = CurrentLeanAngle;
}

// ─── Survival Blend ───────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalBlend(float DeltaSeconds)
{
    // Upper body weight: blend in wounded/exhausted pose when low health or stamina
    float HealthFactor   = 1.0f - FMath::Clamp(HealthNormalized, 0.0f, 1.0f);
    float StaminaFactor  = 1.0f - FMath::Clamp(StaminaNormalized, 0.0f, 1.0f);
    float FearFactor     = FMath::Clamp(FearLevel, 0.0f, 1.0f);

    float TargetWeight = FMath::Max3(HealthFactor * 0.8f, StaminaFactor * 0.5f, FearFactor * 0.3f);
    UpperBodyBlendWeight = FMath::FInterpTo(UpperBodyBlendWeight, TargetWeight, DeltaSeconds, 3.0f);
}

// ─── Foot IK ─────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    // Bone names for foot sockets (standard UE5 Mannequin names)
    static const FName LeftFootBone  = TEXT("foot_l");
    static const FName RightFootBone = TEXT("foot_r");

    FVector LeftFootWorld  = Mesh->GetSocketLocation(LeftFootBone);
    FVector RightFootWorld = Mesh->GetSocketLocation(RightFootBone);

    FVector LeftIKOffset  = TraceFootIK(LeftFootWorld,  DeltaSeconds);
    FVector RightIKOffset = TraceFootIK(RightFootWorld, DeltaSeconds);

    // Smooth interpolation
    FootIKLeftOffset  = FMath::VInterpTo(FootIKLeftOffset,  LeftIKOffset,  DeltaSeconds, FootIKInterpSpeed);
    FootIKRightOffset = FMath::VInterpTo(FootIKRightOffset, RightIKOffset, DeltaSeconds, FootIKInterpSpeed);

    // Pelvis offset: lower pelvis to the lowest foot
    float MinZ = FMath::Min(FootIKLeftOffset.Z, FootIKRightOffset.Z);
    FVector TargetPelvis = FVector(0.0f, 0.0f, FMath::Min(MinZ, 0.0f));
    PelvisOffset = FMath::VInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, PelvisInterpSpeed);
}

FVector UDinoSurvivorAnimInstance::TraceFootIK(const FVector& FootLocation, float DeltaSeconds)
{
    if (!OwnerCharacter) return FVector::ZeroVector;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return FVector::ZeroVector;

    FVector TraceStart = FootLocation + FVector(0.0f, 0.0f, FootIKTraceDistance * 0.5f);
    FVector TraceEnd   = FootLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

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

    if (bHit)
    {
        float ZOffset = HitResult.ImpactPoint.Z - FootLocation.Z;
        return FVector(0.0f, 0.0f, ZOffset);
    }

    return FVector::ZeroVector;
}
