// TranspersonalAnimBlueprint.cpp
// Animation Agent #10 — Cycle PROD_CYCLE_AUTO_20260628_011
// UAnimInstance implementation for the TranspersonalCharacter.
// Drives locomotion blend space, jump states, foot IK, and survival-state overlays.

#include "TranspersonalAnimBlueprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

UTranspersonalAnimBlueprint::UTranspersonalAnimBlueprint(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , GroundSpeed(0.f)
    , MovementDirection(0.f)
    , bIsRunning(false)
    , bIsSprinting(false)
    , bIsCrouching(false)
    , bIsInAir(false)
    , VerticalVelocity(0.f)
    , TimeInAir(0.f)
    , bJustLanded(false)
    , LandImpactAlpha(0.f)
    , bIsInjured(false)
    , InjuryBlendAlpha(0.f)
    , bIsExhausted(false)
    , ExhaustionBlendAlpha(0.f)
    , bIsCarrying(false)
    , CarryBlendAlpha(0.f)
    , LeftFootIKAlpha(0.f)
    , RightFootIKAlpha(0.f)
    , PelvisOffset(FVector::ZeroVector)
    , RunThreshold(200.f)
    , SprintThreshold(450.f)
    , LandImpactDecayRate(3.f)
    , OwnerCharacter(nullptr)
    , OwnerMovement(nullptr)
    , bWasInAir(false)
    , AirTime(0.f)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeInitializeAnimation
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter = Cast<ACharacter>(Pawn);
    if (OwnerCharacter)
    {
        OwnerMovement = OwnerCharacter->GetCharacterMovement();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeUpdateAnimation — called every frame
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimBlueprint::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !OwnerMovement) return;

    UpdateLocomotionState(DeltaSeconds);
    UpdateJumpState(DeltaSeconds);
    UpdateSurvivalOverlays(DeltaSeconds);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateLocomotionState
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimBlueprint::UpdateLocomotionState(float DeltaSeconds)
{
    // Velocity in XY plane
    const FVector Velocity = OwnerCharacter->GetVelocity();
    const FVector VelocityXY(Velocity.X, Velocity.Y, 0.f);
    GroundSpeed = VelocityXY.Size();

    // Movement direction relative to actor facing (for strafing blend)
    if (GroundSpeed > 5.f)
    {
        const FRotator ActorRot = OwnerCharacter->GetActorRotation();
        const FVector ForwardDir = ActorRot.Vector();
        const FVector RightDir = FRotationMatrix(ActorRot).GetScaledAxis(EAxis::Y);

        const FVector NormVel = VelocityXY.GetSafeNormal();
        const float ForwardDot = FVector::DotProduct(ForwardDir, NormVel);
        const float RightDot   = FVector::DotProduct(RightDir, NormVel);

        // Angle in degrees: positive = right strafe, negative = left strafe
        MovementDirection = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
    }
    else
    {
        MovementDirection = 0.f;
    }

    // Locomotion tier flags
    bIsRunning   = (GroundSpeed >= RunThreshold);
    bIsSprinting = (GroundSpeed >= SprintThreshold);
    bIsCrouching = OwnerMovement->IsCrouching();
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateJumpState
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimBlueprint::UpdateJumpState(float DeltaSeconds)
{
    const bool bCurrentlyInAir = OwnerMovement->IsFalling();
    bIsInAir = bCurrentlyInAir;
    VerticalVelocity = OwnerCharacter->GetVelocity().Z;

    if (bCurrentlyInAir)
    {
        AirTime += DeltaSeconds;
        TimeInAir = AirTime;
        bJustLanded = false;
    }
    else
    {
        // Detect landing transition
        if (bWasInAir && !bCurrentlyInAir)
        {
            bJustLanded = true;
            // Scale impact alpha by air time (clamped to 0-1)
            LandImpactAlpha = FMath::Clamp(AirTime / 1.5f, 0.2f, 1.f);
        }
        AirTime = 0.f;
        TimeInAir = 0.f;
    }

    // Decay land impact alpha over time
    if (LandImpactAlpha > 0.f)
    {
        LandImpactAlpha = FMath::Max(0.f, LandImpactAlpha - DeltaSeconds * LandImpactDecayRate);
        if (LandImpactAlpha <= 0.f)
        {
            bJustLanded = false;
        }
    }

    bWasInAir = bCurrentlyInAir;
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateSurvivalOverlays
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimBlueprint::UpdateSurvivalOverlays(float DeltaSeconds)
{
    // Injury overlay — blend in/out smoothly
    const float InjuryTarget = bIsInjured ? 1.f : 0.f;
    InjuryBlendAlpha = FMath::FInterpTo(InjuryBlendAlpha, InjuryTarget, DeltaSeconds, 2.f);

    // Exhaustion overlay
    const float ExhaustTarget = bIsExhausted ? 1.f : 0.f;
    ExhaustionBlendAlpha = FMath::FInterpTo(ExhaustionBlendAlpha, ExhaustTarget, DeltaSeconds, 1.5f);

    // Carry overlay
    const float CarryTarget = bIsCarrying ? 1.f : 0.f;
    CarryBlendAlpha = FMath::FInterpTo(CarryBlendAlpha, CarryTarget, DeltaSeconds, 3.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// SetSurvivalState — called by character/survival system
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimBlueprint::SetSurvivalState(bool bInjured, bool bExhausted, bool bCarrying)
{
    bIsInjured   = bInjured;
    bIsExhausted = bExhausted;
    bIsCarrying  = bCarrying;
}

// ─────────────────────────────────────────────────────────────────────────────
// SetFootIKData — called by FootIKComponent each tick
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimBlueprint::SetFootIKData(
    float LeftAlpha,
    float RightAlpha,
    const FVector& InPelvisOffset)
{
    LeftFootIKAlpha  = FMath::Clamp(LeftAlpha,  0.f, 1.f);
    RightFootIKAlpha = FMath::Clamp(RightAlpha, 0.f, 1.f);
    PelvisOffset     = InPelvisOffset;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetLocomotionBlendParams — convenience for Blueprint blend space queries
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimBlueprint::GetLocomotionBlendParams(float& OutSpeed, float& OutDirection) const
{
    OutSpeed     = GroundSpeed;
    OutDirection = MovementDirection;
}
