// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full AnimInstance implementation: locomotion states, foot IK, blend spaces

#include "PlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    Direction = 0.0f;
    LeanAngle = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsAiming = false;
    bIsAttacking = false;

    // Foot IK defaults
    bEnableFootIK = true;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    PelvisOffset = 0.0f;

    // State defaults
    LocomotionState = EAnim_LocomotionState::Idle;
    WeaponState = EAnim_WeaponState::Unarmed;

    // Blend weights
    UpperBodyBlendWeight = 0.0f;
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    // Jump
    JumpStartTime = 0.0f;
    bJustLanded = false;
    LandImpactAlpha = 0.0f;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        unreal::log_message("PlayerAnimInstance initialized for: " + OwnerCharacter->GetName());
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        return;
    }

    UpdateLocomotionData(DeltaSeconds);
    UpdateAimData(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateLocomotionState();
    UpdateLandImpact(DeltaSeconds);
}

// ─── LOCOMOTION ───────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    // Velocity-based speed
    FVector Velocity = OwnerCharacter->GetVelocity();
    Speed = Velocity.Size2D();

    // Direction — angle between velocity and forward vector
    if (Speed > 1.0f)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FVector ForwardVec = ActorRot.Vector();
        FVector VelocityNorm = Velocity.GetSafeNormal2D();
        Direction = UKismetMathLibrary::DegAcos(FVector::DotProduct(ForwardVec, VelocityNorm));

        // Determine sign (left/right)
        FVector RightVec = FVector::CrossProduct(FVector::UpVector, ForwardVec);
        if (FVector::DotProduct(RightVec, VelocityNorm) < 0.0f)
        {
            Direction = -Direction;
        }
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.0f, DeltaSeconds, 5.0f);
    }

    // Lean angle — based on angular velocity
    float AngularVelocityZ = MovementComponent->GetLastUpdateRotation().Yaw;
    LeanAngle = FMath::FInterpTo(LeanAngle, AngularVelocityZ * 0.05f, DeltaSeconds, 8.0f);
    LeanAngle = FMath::Clamp(LeanAngle, -15.0f, 15.0f);

    // Movement state flags
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsSprinting = Speed > 400.0f && !bIsInAir;
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
    EAnim_LocomotionState NewState = EAnim_LocomotionState::Idle;

    if (bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
            NewState = EAnim_LocomotionState::Jump;
        else
            NewState = EAnim_LocomotionState::Fall;
    }
    else if (bJustLanded)
    {
        NewState = EAnim_LocomotionState::Land;
    }
    else if (bIsCrouching)
    {
        NewState = (Speed > 10.0f) ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }
    else if (Speed > 400.0f)
    {
        NewState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed > 150.0f)
    {
        NewState = EAnim_LocomotionState::Run;
    }
    else if (Speed > 10.0f)
    {
        NewState = EAnim_LocomotionState::Walk;
    }
    else
    {
        NewState = EAnim_LocomotionState::Idle;
    }

    LocomotionState = NewState;
}

// ─── AIM DATA ─────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateAimData(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::FInterpTo(AimPitch, FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f), DeltaSeconds, 15.0f);
    AimYaw = FMath::FInterpTo(AimYaw, FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f), DeltaSeconds, 15.0f);

    // Upper body blend — only when aiming or attacking
    float TargetUpperBlend = (bIsAiming || bIsAttacking) ? 1.0f : 0.0f;
    UpperBodyBlendWeight = FMath::FInterpTo(UpperBodyBlendWeight, TargetUpperBlend, DeltaSeconds, 10.0f);
}

// ─── FOOT IK ──────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!bEnableFootIK || !OwnerCharacter) return;

    // Trace for left foot
    FAnim_FootIKData NewLeftFoot = TraceFootIK(FName("foot_l"), DeltaSeconds);
    FAnim_FootIKData NewRightFoot = TraceFootIK(FName("foot_r"), DeltaSeconds);

    // Smooth interpolation
    LeftFootIK.FootLocation = FMath::VInterpTo(LeftFootIK.FootLocation, NewLeftFoot.FootLocation, DeltaSeconds, FootIKInterpSpeed);
    LeftFootIK.FootRotation = FMath::RInterpTo(LeftFootIK.FootRotation, NewLeftFoot.FootRotation, DeltaSeconds, FootIKInterpSpeed);
    LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, NewLeftFoot.IKAlpha, DeltaSeconds, FootIKInterpSpeed);

    RightFootIK.FootLocation = FMath::VInterpTo(RightFootIK.FootLocation, NewRightFoot.FootLocation, DeltaSeconds, FootIKInterpSpeed);
    RightFootIK.FootRotation = FMath::RInterpTo(RightFootIK.FootRotation, NewRightFoot.FootRotation, DeltaSeconds, FootIKInterpSpeed);
    RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, NewRightFoot.IKAlpha, DeltaSeconds, FootIKInterpSpeed);

    // Pelvis offset — lower pelvis to accommodate the lower foot
    float LowestFoot = FMath::Min(LeftFootIK.FootLocation.Z, RightFootIK.FootLocation.Z);
    float TargetPelvis = FMath::Clamp(LowestFoot, -30.0f, 0.0f);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, FootIKInterpSpeed * 0.5f);
}

FAnim_FootIKData UPlayerAnimInstance::TraceFootIK(FName SocketName, float DeltaSeconds)
{
    FAnim_FootIKData Result;
    Result.IKAlpha = 0.0f;

    if (!OwnerCharacter) return Result;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return Result;

    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
    FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

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

    if (bHit)
    {
        // Offset from socket to hit point
        Result.FootLocation = FVector(0.0f, 0.0f, HitResult.ImpactPoint.Z - SocketLocation.Z);

        // Foot rotation from surface normal
        FVector Normal = HitResult.ImpactNormal;
        FRotator FootRot = UKismetMathLibrary::MakeRotFromZX(Normal, OwnerCharacter->GetActorForwardVector());
        Result.FootRotation = FRotator(FootRot.Pitch, 0.0f, FootRot.Roll);

        Result.IKAlpha = 1.0f;
    }

    return Result;
}

// ─── LAND IMPACT ──────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateLandImpact(float DeltaSeconds)
{
    if (bJustLanded)
    {
        LandImpactAlpha = FMath::FInterpTo(LandImpactAlpha, 1.0f, DeltaSeconds, 20.0f);
        if (LandImpactAlpha > 0.95f)
        {
            bJustLanded = false;
        }
    }
    else
    {
        LandImpactAlpha = FMath::FInterpTo(LandImpactAlpha, 0.0f, DeltaSeconds, 5.0f);
    }
}

void UPlayerAnimInstance::OnLanded()
{
    bJustLanded = true;
    LandImpactAlpha = 0.0f;
}

// ─── STATE SETTERS (called from Character) ────────────────────────────────────

void UPlayerAnimInstance::SetSprinting(bool bSprint)
{
    bIsSprinting = bSprint;
}

void UPlayerAnimInstance::SetAiming(bool bAim)
{
    bIsAiming = bAim;
}

void UPlayerAnimInstance::SetAttacking(bool bAttack)
{
    bIsAttacking = bAttack;
}

void UPlayerAnimInstance::SetWeaponState(EAnim_WeaponState NewWeaponState)
{
    WeaponState = NewWeaponState;
}

void UPlayerAnimInstance::SetCrouching(bool bCrouch)
{
    bIsCrouching = bCrouch;
}
