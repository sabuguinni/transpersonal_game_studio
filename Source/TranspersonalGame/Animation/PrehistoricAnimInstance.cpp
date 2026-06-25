// PrehistoricAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Locomotion AnimInstance for the prehistoric survivor character.
// Drives idle/walk/run/crouch/jump blend states from character movement data.

#include "PrehistoricAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UPrehistoricAnimInstance::UPrehistoricAnimInstance()
{
    Speed            = 0.f;
    Direction        = 0.f;
    bIsInAir         = false;
    bIsCrouching     = false;
    bIsSprinting     = false;
    bIsAttacking     = false;
    bIsDead          = false;
    LeanAngle        = 0.f;
    AimPitch         = 0.f;
    LandedBlendAlpha = 0.f;
    FearIntensity    = 0.f;
    StaminaRatio     = 1.f;
}

void UPrehistoricAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UPrehistoricAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (!OwnerCharacter) return;
    }

    UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    // ── Velocity & Speed ────────────────────────────────────────────────────
    FVector Velocity = MovComp->Velocity;
    Speed = Velocity.Size2D();

    // ── Direction (strafe angle relative to actor forward) ─────────────────
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator VelRot   = UKismetMathLibrary::MakeRotFromX(Velocity);
    FRotator Delta    = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot);
    Direction = FMath::FInterpTo(Direction, Delta.Yaw, DeltaSeconds, 8.f);

    // ── Air state ───────────────────────────────────────────────────────────
    bIsInAir = MovComp->IsFalling();

    // ── Crouch ──────────────────────────────────────────────────────────────
    bIsCrouching = MovComp->IsCrouching();

    // ── Sprint (speed threshold: >400 = sprinting) ──────────────────────────
    bIsSprinting = (Speed > 400.f && !bIsInAir && !bIsCrouching);

    // ── Lean (lateral tilt based on direction angle) ────────────────────────
    float TargetLean = FMath::Clamp(Direction * 0.15f, -15.f, 15.f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 5.f);

    // ── Aim pitch (look up/down) ─────────────────────────────────────────────
    FRotator ControlRot = OwnerCharacter->GetControlRotation();
    AimPitch = FMath::ClampAngle(ControlRot.Pitch, -90.f, 90.f);

    // ── Land blend alpha (smoothly fade out landing pose) ──────────────────
    if (!bIsInAir && LandedBlendAlpha > 0.f)
    {
        LandedBlendAlpha = FMath::FInterpTo(LandedBlendAlpha, 0.f, DeltaSeconds, 4.f);
    }

    // ── Stamina ratio drives animation speed scale ──────────────────────────
    // StaminaRatio is set externally by the character's survival component.
    // When stamina is low, movement animations slow slightly.
    float AnimSpeedScale = FMath::Lerp(0.75f, 1.0f, StaminaRatio);
    SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
    (void)AnimSpeedScale; // Used in Blueprint via GetAnimSpeedScale()
}

void UPrehistoricAnimInstance::OnLanded()
{
    LandedBlendAlpha = 1.0f;
}

void UPrehistoricAnimInstance::SetAttacking(bool bAttacking)
{
    bIsAttacking = bAttacking;
}

void UPrehistoricAnimInstance::SetDead(bool bDead)
{
    bIsDead = bDead;
}

void UPrehistoricAnimInstance::SetFearIntensity(float Intensity)
{
    FearIntensity = FMath::Clamp(Intensity, 0.f, 1.f);
}

void UPrehistoricAnimInstance::SetStaminaRatio(float Ratio)
{
    StaminaRatio = FMath::Clamp(Ratio, 0.f, 1.f);
}

float UPrehistoricAnimInstance::GetAnimSpeedScale() const
{
    return FMath::Lerp(0.75f, 1.0f, StaminaRatio);
}
