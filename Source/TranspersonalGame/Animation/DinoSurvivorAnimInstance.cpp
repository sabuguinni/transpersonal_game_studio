// DinoSurvivorAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Implements the AnimInstance for the prehistoric survivor character.
// Handles locomotion state machine, foot IK, aim offset, and survival state blending.

#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    // Locomotion defaults
    GroundSpeed = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    MovementDirection = 0.0f;
    LeanAngle = 0.0f;
    LocomotionState = EAnim_LocomotionState::Idle;
    CombatStance = EAnim_CombatStance::Unarmed;

    // Foot IK defaults
    FootIKData.LeftFootLocation = FVector::ZeroVector;
    FootIKData.RightFootLocation = FVector::ZeroVector;
    FootIKData.LeftFootRotation = FRotator::ZeroRotator;
    FootIKData.RightFootRotation = FRotator::ZeroRotator;
    FootIKData.LeftFootAlpha = 0.0f;
    FootIKData.RightFootAlpha = 0.0f;
    FootIKData.PelvisOffset = 0.0f;
    FootIKData.bLeftFootOnGround = false;
    FootIKData.bRightFootOnGround = false;

    // Aim offset defaults
    AimPitch = 0.0f;
    AimYaw = 0.0f;
    AimOffsetAlpha = 0.0f;

    // Survival state defaults
    StaminaNormalized = 1.0f;
    HealthNormalized = 1.0f;
    FearLevel = 0.0f;
    bIsExhausted = false;
    bIsWounded = false;
    bIsFleeing = false;

    // Blend weights
    AdditiveLayerWeight = 1.0f;
    UpperBodyWeight = 1.0f;

    // Foot IK config
    FootIKTraceDistance = 55.0f;
    FootIKInterpSpeed = 15.0f;
    PelvisInterpSpeed = 10.0f;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (!OwnerCharacter) return;
    }

    UpdateLocomotionData(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateSurvivalState(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateLocomotionState();
}

void UDinoSurvivorAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    // Ground speed (horizontal only)
    FVector Velocity = MovComp->Velocity;
    GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();

    bIsInAir = MovComp->IsFalling();
    bIsCrouching = MovComp->IsCrouching();

    // Sprint detection: speed > 400 units/s
    bIsSprinting = (GroundSpeed > 400.0f) && !bIsInAir && !bIsCrouching;

    // Movement direction relative to actor forward
    if (GroundSpeed > 5.0f)
    {
        FVector ActorForward = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDir = Velocity.GetSafeNormal2D();
        float DotForward = FVector::DotProduct(ActorForward, VelocityDir);
        float DotRight = FVector::DotProduct(OwnerCharacter->GetActorRightVector(), VelocityDir);
        MovementDirection = FMath::RadiansToDegrees(FMath::Atan2(DotRight, DotForward));
    }
    else
    {
        MovementDirection = FMath::FInterpTo(MovementDirection, 0.0f, DeltaSeconds, 5.0f);
    }

    // Lean angle (lateral acceleration)
    FVector Accel = MovComp->GetCurrentAcceleration();
    float LateralAccel = FVector::DotProduct(OwnerCharacter->GetActorRightVector(), Accel);
    float TargetLean = FMath::Clamp(LateralAccel / 600.0f, -1.0f, 1.0f) * 15.0f;
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 6.0f);
}

void UDinoSurvivorAnimInstance::UpdateLocomotionState()
{
    if (bIsInAir)
    {
        if (OwnerCharacter && OwnerCharacter->GetCharacterMovement()->Velocity.Z > 0.0f)
            LocomotionState = EAnim_LocomotionState::Jump;
        else
            LocomotionState = EAnim_LocomotionState::Fall;
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = (GroundSpeed > 10.0f) ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
        return;
    }

    if (GroundSpeed < 10.0f)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (bIsSprinting)
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

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    float TargetPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
    float TargetYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);

    AimPitch = FMath::FInterpTo(AimPitch, TargetPitch, DeltaSeconds, 15.0f);
    AimYaw = FMath::FInterpTo(AimYaw, TargetYaw, DeltaSeconds, 15.0f);

    // Aim offset active when not sprinting and not in air
    float TargetAimAlpha = (!bIsSprinting && !bIsInAir) ? 1.0f : 0.0f;
    AimOffsetAlpha = FMath::FInterpTo(AimOffsetAlpha, TargetAimAlpha, DeltaSeconds, 8.0f);
}

void UDinoSurvivorAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
    // Exhaustion affects animation blend
    bIsExhausted = (StaminaNormalized < 0.15f);
    bIsWounded = (HealthNormalized < 0.25f);

    // Fear-based flee state
    bIsFleeing = (FearLevel > 0.75f) && (GroundSpeed > 50.0f);

    // Upper body weight: reduce when exhausted or fleeing
    float TargetUpperWeight = 1.0f;
    if (bIsExhausted) TargetUpperWeight = 0.4f;
    else if (bIsFleeing) TargetUpperWeight = 0.6f;
    UpperBodyWeight = FMath::FInterpTo(UpperBodyWeight, TargetUpperWeight, DeltaSeconds, 4.0f);
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;
    if (bIsInAir)
    {
        // Lift feet when airborne
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    // Bone socket approximate positions (offset from actor root)
    FVector ActorLoc = OwnerCharacter->GetActorLocation();
    FVector LeftFootStart = ActorLoc + OwnerCharacter->GetActorRightVector() * -25.0f;
    FVector RightFootStart = ActorLoc + OwnerCharacter->GetActorRightVector() * 25.0f;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    auto TraceFootIK = [&](FVector Start, FAnim_FootIKData& IKData, bool bLeft) -> void
    {
        FVector TraceEnd = Start - FVector(0, 0, FootIKTraceDistance);
        FHitResult Hit;
        bool bHit = World->LineTraceSingleByChannel(Hit, Start, TraceEnd, ECC_Visibility, Params);

        float& Alpha = bLeft ? IKData.LeftFootAlpha : IKData.RightFootAlpha;
        FVector& FootLoc = bLeft ? IKData.LeftFootLocation : IKData.RightFootLocation;
        FRotator& FootRot = bLeft ? IKData.LeftFootRotation : IKData.RightFootRotation;
        bool& bOnGround = bLeft ? IKData.bLeftFootOnGround : IKData.bRightFootOnGround;

        if (bHit)
        {
            bOnGround = true;
            float TargetAlpha = 1.0f;
            Alpha = FMath::FInterpTo(Alpha, TargetAlpha, DeltaSeconds, FootIKInterpSpeed);

            // Foot location offset from capsule base
            float HeightOffset = Hit.Location.Z - (ActorLoc.Z - OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
            FVector TargetLoc = FVector(0, 0, HeightOffset);
            FootLoc = FMath::VInterpTo(FootLoc, TargetLoc, DeltaSeconds, FootIKInterpSpeed);

            // Foot rotation from surface normal
            FRotator SurfaceRot = UKismetMathLibrary::MakeRotFromZX(Hit.Normal, OwnerCharacter->GetActorForwardVector());
            FootRot = FMath::RInterpTo(FootRot, SurfaceRot, DeltaSeconds, FootIKInterpSpeed);
        }
        else
        {
            bOnGround = false;
            Alpha = FMath::FInterpTo(Alpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        }
    };

    TraceFootIK(LeftFootStart, FootIKData, true);
    TraceFootIK(RightFootStart, FootIKData, false);

    // Pelvis offset: lower pelvis to accommodate the lower foot
    float LowestFoot = FMath::Min(FootIKData.LeftFootLocation.Z, FootIKData.RightFootLocation.Z);
    float TargetPelvis = FMath::Clamp(LowestFoot, -30.0f, 0.0f);
    FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, TargetPelvis, DeltaSeconds, PelvisInterpSpeed);
}
