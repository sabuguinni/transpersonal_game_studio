// TranspersonalAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Prehistoric survival game character animation instance
// Implements: locomotion blend, foot IK, state machine transitions

#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsAccelerating = false;
    bIsSprinting = false;

    // Foot IK defaults
    bEnableFootIK = true;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;
    PelvisOffset = FVector::ZeroVector;

    // State defaults
    AnimState = EAnim_CharacterState::Idle;
    PreviousAnimState = EAnim_CharacterState::Idle;
    StateBlendAlpha = 1.0f;

    // Survival stats
    StaminaNormalized = 1.0f;
    FearLevel = 0.0f;
    bIsExhausted = false;
    bIsWounded = false;
    WoundSeverity = 0.0f;

    // Lean
    LeanAmount = 0.0f;
    AimPitch = 0.0f;
    AimYaw = 0.0f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
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
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (OwnerCharacter)
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        return;
    }

    UpdateLocomotionData(DeltaSeconds);
    UpdateAnimationState(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalAnimations(DeltaSeconds);
    UpdateLeanAndAim(DeltaSeconds);
}

void UTranspersonalAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    FVector Velocity = OwnerCharacter->GetVelocity();
    Speed = Velocity.Size2D();

    // Direction relative to character facing
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;

    // Sprint detection — speed above walk threshold
    const float SprintThreshold = 400.0f;
    bIsSprinting = Speed > SprintThreshold && !bIsInAir;
}

void UTranspersonalAnimInstance::UpdateAnimationState(float DeltaSeconds)
{
    PreviousAnimState = AnimState;

    if (bIsInAir)
    {
        AnimState = EAnim_CharacterState::Jump;
    }
    else if (bIsCrouching)
    {
        AnimState = Speed > 10.0f ? EAnim_CharacterState::CrouchWalk : EAnim_CharacterState::Crouch;
    }
    else if (Speed < 10.0f)
    {
        AnimState = EAnim_CharacterState::Idle;
    }
    else if (bIsSprinting)
    {
        AnimState = EAnim_CharacterState::Sprint;
    }
    else if (Speed < 200.0f)
    {
        AnimState = EAnim_CharacterState::Walk;
    }
    else
    {
        AnimState = EAnim_CharacterState::Run;
    }

    // Blend alpha for smooth transitions
    if (AnimState != PreviousAnimState)
    {
        StateBlendAlpha = 0.0f;
    }
    StateBlendAlpha = FMath::FInterpTo(StateBlendAlpha, 1.0f, DeltaSeconds, 8.0f);
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!bEnableFootIK || !OwnerCharacter)
        return;

    // Trace for left foot
    FVector LeftFootWorld = GetOwningComponent()->GetSocketLocation(FName("foot_l"));
    FVector RightFootWorld = GetOwningComponent()->GetSocketLocation(FName("foot_r"));

    FVector TraceStart_L = LeftFootWorld + FVector(0, 0, FootIKTraceDistance);
    FVector TraceEnd_L = LeftFootWorld - FVector(0, 0, FootIKTraceDistance);

    FVector TraceStart_R = RightFootWorld + FVector(0, 0, FootIKTraceDistance);
    FVector TraceEnd_R = RightFootWorld - FVector(0, 0, FootIKTraceDistance);

    FHitResult HitL, HitR;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    UWorld* World = GetWorld();
    if (!World) return;

    bool bHitL = World->LineTraceSingleByChannel(HitL, TraceStart_L, TraceEnd_L, ECC_Visibility, Params);
    bool bHitR = World->LineTraceSingleByChannel(HitR, TraceStart_R, TraceEnd_R, ECC_Visibility, Params);

    // Calculate offsets
    FVector TargetOffsetL = FVector::ZeroVector;
    FVector TargetOffsetR = FVector::ZeroVector;

    if (bHitL)
    {
        float HeightDiff = HitL.ImpactPoint.Z - LeftFootWorld.Z;
        TargetOffsetL = FVector(0, 0, HeightDiff);
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    if (bHitR)
    {
        float HeightDiff = HitR.ImpactPoint.Z - RightFootWorld.Z;
        TargetOffsetR = FVector(0, 0, HeightDiff);
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
    }
    else
    {
        RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
    }

    // Smooth interpolation of foot offsets
    LeftFootOffset = FMath::VInterpTo(LeftFootOffset, TargetOffsetL, DeltaSeconds, FootIKInterpSpeed);
    RightFootOffset = FMath::VInterpTo(RightFootOffset, TargetOffsetR, DeltaSeconds, FootIKInterpSpeed);

    // Pelvis compensation — lower pelvis to reach lower foot
    float MinFootZ = FMath::Min(LeftFootOffset.Z, RightFootOffset.Z);
    FVector TargetPelvis = FVector(0, 0, FMath::Min(0.0f, MinFootZ));
    PelvisOffset = FMath::VInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, FootIKInterpSpeed * 0.5f);
}

void UTranspersonalAnimInstance::UpdateSurvivalAnimations(float DeltaSeconds)
{
    // Exhaustion affects movement blend
    if (bIsExhausted)
    {
        // Reduce effective speed for animation blending
        Speed = FMath::Min(Speed, 200.0f); // Cap at walk speed when exhausted
    }

    // Wound severity affects posture
    if (bIsWounded && WoundSeverity > 0.5f)
    {
        // Wounded characters lean and move differently
        // This feeds into the blend space for wounded locomotion
        LeanAmount = FMath::FInterpTo(LeanAmount, WoundSeverity * 15.0f, DeltaSeconds, 2.0f);
    }

    // Fear affects animation speed multiplier (trembling, cautious movement)
    if (FearLevel > 0.7f)
    {
        // High fear = crouched, cautious movement
        // This is handled by the state machine via bIsCrouching override
    }
}

void UTranspersonalAnimInstance::UpdateLeanAndAim(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Calculate lean from lateral acceleration
    FVector Accel = MovementComponent->GetCurrentAcceleration();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector LocalAccel = ActorRot.UnrotateVector(Accel);

    float TargetLean = FMath::Clamp(LocalAccel.Y / MovementComponent->MaxAcceleration * 15.0f, -15.0f, 15.0f);
    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, 5.0f);

    // Aim offset from controller rotation
    FRotator ControlRot = OwnerCharacter->GetControlRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.0f, 90.0f);
}

void UTranspersonalAnimInstance::SetSurvivalStats(float Stamina, float Fear, bool bWounded, float WoundLevel)
{
    StaminaNormalized = FMath::Clamp(Stamina, 0.0f, 1.0f);
    FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);
    bIsWounded = bWounded;
    WoundSeverity = FMath::Clamp(WoundLevel, 0.0f, 1.0f);
    bIsExhausted = StaminaNormalized < 0.1f;
}

void UTranspersonalAnimInstance::TriggerAttackMontage(EAnim_AttackType AttackType)
{
    // Attack montage triggering — montage asset assigned in Blueprint
    AnimState = EAnim_CharacterState::Attack;
    StateBlendAlpha = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("TranspersonalAnimInstance: Attack triggered — type %d"), (int32)AttackType);
}

void UTranspersonalAnimInstance::TriggerDeathMontage()
{
    AnimState = EAnim_CharacterState::Death;
    StateBlendAlpha = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("TranspersonalAnimInstance: Death triggered"));
}

float UTranspersonalAnimInstance::GetLocomotionBlendWeight() const
{
    // Returns 0-1 blend between idle and locomotion
    return FMath::Clamp(Speed / 600.0f, 0.0f, 1.0f);
}

bool UTranspersonalAnimInstance::ShouldUseSprintAnimation() const
{
    return bIsSprinting && !bIsExhausted && !bIsWounded;
}
