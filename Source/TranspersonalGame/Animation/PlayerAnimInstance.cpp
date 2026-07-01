// Copyright Transpersonal Game Studio. All Rights Reserved.
// PlayerAnimInstance.cpp — Animation Agent #10

#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Defaults are set in the header via member initializers
}

// ─────────────────────────────────────────────────────────────────────────────
// UAnimInstance interface
// ─────────────────────────────────────────────────────────────────────────────

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
        // Try to re-cache if character wasn't ready at init
        OwnerCharacter = Cast<ACharacter>(GetOwningActor());
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        return;
    }

    // Update all animation data
    UpdateLocomotionState();
    UpdateSurvivalAnimations(DeltaSeconds);
    UpdateAimOffset();

    // Landing timer countdown
    if (bLandingActive)
    {
        LandingTimer -= DeltaSeconds;
        if (LandingTimer <= 0.0f)
        {
            bLandingActive = false;
            LandingTimer = 0.0f;
        }
    }
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // Foot IK runs thread-safe for performance
    UpdateFootIK(DeltaSeconds);
}

// ─────────────────────────────────────────────────────────────────────────────
// Locomotion
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (!OwnerCharacter || !MovementComponent) return;

    const FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;

    bIsInAir    = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Direction (strafe angle relative to character facing)
    if (Speed > WalkSpeedThreshold)
    {
        const FRotator ActorRot = OwnerCharacter->GetActorRotation();
        const FRotator VelocityRot = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // Lean angle — proportional to direction for natural body lean
    LeanAngle = FMath::Clamp(Direction * 0.3f, -25.0f, 25.0f);

    // Determine locomotion state
    if (bLandingActive)
    {
        LocomotionState = EAnim_LocomotionState::Land;
    }
    else if (bIsInAir)
    {
        LocomotionState = VerticalVelocity > 0.0f
            ? EAnim_LocomotionState::Jump
            : EAnim_LocomotionState::Fall;
    }
    else if (bIsCrouching)
    {
        LocomotionState = Speed > WalkSpeedThreshold
            ? EAnim_LocomotionState::CrouchWalk
            : EAnim_LocomotionState::Crouch;
    }
    else if (Speed >= SprintSpeedThreshold && bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed >= RunSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else if (Speed >= WalkSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Foot IK
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    // Only apply foot IK when grounded
    if (bIsInAir || !OwnerCharacter) return;

    SolveFootIK(FName("foot_l"), LeftFootIK, DeltaSeconds);
    SolveFootIK(FName("foot_r"), RightFootIK, DeltaSeconds);

    // Pelvis offset = lowest foot offset to prevent clipping
    PelvisOffset = FMath::Min(LeftFootIK.PelvisOffset, RightFootIK.PelvisOffset);
}

void UPlayerAnimInstance::SolveFootIK(const FName& FootSocketName,
                                       FAnim_FootIKData& OutFootData,
                                       float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    const FVector FootWorldLocation = Mesh->GetSocketLocation(FootSocketName);
    const FVector TraceStart = FootWorldLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
    const FVector TraceEnd   = FootWorldLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Target foot location in component space
        const FVector TargetLocation = Mesh->GetComponentTransform().InverseTransformPosition(HitResult.Location);

        // Smooth interpolation
        OutFootData.FootLocation = FMath::VInterpTo(
            OutFootData.FootLocation,
            TargetLocation,
            DeltaSeconds,
            FootIKInterpSpeed
        );

        // Surface normal → foot rotation
        const FRotator SurfaceRotation = HitResult.Normal.Rotation();
        const FRotator TargetRotation  = FRotator(-SurfaceRotation.Pitch, 0.0f, SurfaceRotation.Roll);
        OutFootData.FootRotation = FMath::RInterpTo(
            OutFootData.FootRotation,
            TargetRotation,
            DeltaSeconds,
            FootIKInterpSpeed
        );

        // Pelvis offset — how much to lower the pelvis
        OutFootData.PelvisOffset = HitResult.Location.Z - FootWorldLocation.Z;
        OutFootData.IKAlpha = 1.0f;
    }
    else
    {
        // No ground hit — smoothly disable IK
        OutFootData.IKAlpha = FMath::FInterpTo(OutFootData.IKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        OutFootData.PelvisOffset = FMath::FInterpTo(OutFootData.PelvisOffset, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival-driven animations
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateSurvivalAnimations(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Try to get survival stats from the character
    // These are read via generic property access to avoid hard coupling
    // The TranspersonalCharacter exposes Health, Fear, Stamina as floats 0-100
    
    // Default safe values if stats not available
    float Health  = 100.0f;
    float Fear    = 0.0f;
    float Stamina = 100.0f;

    // Attempt to read from character properties (Blueprint-exposed)
    // Using FindProperty for loose coupling
    if (FFloatProperty* HealthProp = FindFProperty<FFloatProperty>(
            OwnerCharacter->GetClass(), FName("Health")))
    {
        Health = HealthProp->GetPropertyValue_InContainer(OwnerCharacter);
    }
    if (FFloatProperty* FearProp = FindFProperty<FFloatProperty>(
            OwnerCharacter->GetClass(), FName("Fear")))
    {
        Fear = FearProp->GetPropertyValue_InContainer(OwnerCharacter);
    }
    if (FFloatProperty* StaminaProp = FindFProperty<FFloatProperty>(
            OwnerCharacter->GetClass(), FName("Stamina")))
    {
        Stamina = StaminaProp->GetPropertyValue_InContainer(OwnerCharacter);
    }

    // Compute alphas (0-1 range)
    const float TargetInjury     = FMath::Clamp(1.0f - (Health  / 100.0f), 0.0f, 1.0f);
    const float TargetFear       = FMath::Clamp(Fear   / 100.0f, 0.0f, 1.0f);
    const float TargetExhaustion = FMath::Clamp(1.0f - (Stamina / 100.0f), 0.0f, 1.0f);

    // Smooth transitions so animations don't snap
    InjuryAlpha     = FMath::FInterpTo(InjuryAlpha,     TargetInjury,     DeltaSeconds, 2.0f);
    FearAlpha       = FMath::FInterpTo(FearAlpha,       TargetFear,       DeltaSeconds, 3.0f);
    ExhaustionAlpha = FMath::FInterpTo(ExhaustionAlpha, TargetExhaustion, DeltaSeconds, 1.5f);

    // Sprint detection — exhausted characters cannot sprint
    bIsSprinting = (Speed >= SprintSpeedThreshold) && (ExhaustionAlpha < 0.8f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Aim offset
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateAimOffset()
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    AimRotation = Controller->GetControlRotation();

    // Relative to character rotation for aim offset blendspace
    const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, ActorRotation);

    AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::Clamp(DeltaRot.Yaw,   -90.0f, 90.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public functions
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::SetWeaponState(EAnim_WeaponState NewWeaponState)
{
    WeaponState = NewWeaponState;
}

void UPlayerAnimInstance::TriggerLandingResponse()
{
    if (!bLandingActive)
    {
        bLandingActive = true;
        LandingTimer   = LandingDuration;
    }
}

bool UPlayerAnimInstance::IsMoving() const
{
    return Speed > WalkSpeedThreshold;
}
