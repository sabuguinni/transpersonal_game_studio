// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260630_003
// Full implementation of UPlayerAnimInstance — locomotion state machine,
// IK foot placement, blend space updates, montage management.

#include "PlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    SmoothedSpeed = 0.0f;
    Direction = 0.0f;
    LeanAmount = 0.0f;
    VerticalVelocity = 0.0f;

    bIsMoving = false;
    bIsSprinting = false;
    bIsCrouching = false;
    bIsInAir = false;
    bIsSneaking = false;
    bIsClimbing = false;

    LocomotionState = EAnim_LocomotionState::Idle;
    CombatStance = EAnim_CombatStance::Unarmed;

    // IK defaults
    LeftFootIKAlpha = 0.0f;
    RightFootIKAlpha = 0.0f;
    LeftFootOffset = FVector::ZeroVector;
    RightFootOffset = FVector::ZeroVector;
    PelvisOffset = FVector::ZeroVector;
    bEnableFootIK = true;

    // Survival state defaults
    StaminaNormalized = 1.0f;
    HealthNormalized = 1.0f;
    FearLevel = 0.0f;
    bIsExhausted = false;
    bIsWounded = false;

    // Aim defaults
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    // Blend weights
    AdditiveLayerWeight = 0.0f;
    UpperBodyWeight = 0.0f;

    // Smoothing
    SpeedSmoothingRate = 10.0f;
    LeanSmoothingRate = 5.0f;
    IKSmoothingRate = 15.0f;

    // IK trace config
    IKTraceDistance = 50.0f;
    IKTraceRadius = 15.0f;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
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
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (OwnerCharacter)
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        return;
    }

    UpdateLocomotionData(DeltaSeconds);
    UpdateLocomotionState();
    UpdateAimData();
    UpdateSurvivalState();

    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaSeconds);
    }
}

void UPlayerAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    FVector Velocity = MovementComponent->Velocity;
    float RawSpeed = Velocity.Size2D();

    // Smooth speed
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, RawSpeed, DeltaSeconds, SpeedSmoothingRate);
    Speed = SmoothedSpeed;

    // Vertical velocity (for jump/fall blending)
    VerticalVelocity = Velocity.Z;

    // Direction angle relative to character facing
    FRotator CharRot = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = CharRot.UnrotateVector(Velocity);
    Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

    // Lean — lateral acceleration-based
    FVector Acceleration = MovementComponent->GetCurrentAcceleration();
    FVector LocalAccel = CharRot.UnrotateVector(Acceleration);
    float TargetLean = FMath::Clamp(LocalAccel.Y * 0.02f, -1.0f, 1.0f);
    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, LeanSmoothingRate);

    // Boolean states
    bIsMoving = RawSpeed > 10.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bIsSprinting = RawSpeed > 400.0f && bIsMoving && !bIsCrouching;
    bIsSneaking = bIsCrouching && bIsMoving;
    bIsClimbing = (MovementComponent->MovementMode == MOVE_Custom);
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
    EAnim_LocomotionState NewState = LocomotionState;

    if (bIsInAir)
    {
        NewState = (VerticalVelocity > 0.0f)
            ? EAnim_LocomotionState::Jump
            : EAnim_LocomotionState::Fall;
    }
    else if (bIsClimbing)
    {
        NewState = EAnim_LocomotionState::Climb;
    }
    else if (bIsCrouching && bIsMoving)
    {
        NewState = EAnim_LocomotionState::Sneak;
    }
    else if (bIsSprinting)
    {
        NewState = EAnim_LocomotionState::Sprint;
    }
    else if (bIsMoving)
    {
        NewState = (Speed > 200.0f)
            ? EAnim_LocomotionState::Run
            : EAnim_LocomotionState::Walk;
    }
    else
    {
        NewState = EAnim_LocomotionState::Idle;
    }

    LocomotionState = NewState;
}

void UPlayerAnimInstance::UpdateAimData()
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::Clamp(Delta.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::Clamp(Delta.Yaw, -90.0f, 90.0f);
}

void UPlayerAnimInstance::UpdateSurvivalState()
{
    // These values are driven externally by TranspersonalCharacter survival stats.
    // Here we derive animation consequences from them.
    bIsExhausted = StaminaNormalized < 0.15f;
    bIsWounded = HealthNormalized < 0.30f;

    // Additive layer weight: exhausted/wounded characters get additive body sway
    float TargetAdditive = 0.0f;
    if (bIsExhausted) TargetAdditive = FMath::Max(TargetAdditive, 0.6f);
    if (bIsWounded) TargetAdditive = FMath::Max(TargetAdditive, 0.8f);
    if (FearLevel > 0.7f) TargetAdditive = FMath::Max(TargetAdditive, 0.4f);

    AdditiveLayerWeight = FMath::FInterpTo(AdditiveLayerWeight, TargetAdditive, GetWorld()->GetDeltaSeconds(), 3.0f);

    // Upper body weight for weapon/tool holding
    float TargetUpper = (CombatStance != EAnim_CombatStance::Unarmed) ? 1.0f : 0.0f;
    UpperBodyWeight = FMath::FInterpTo(UpperBodyWeight, TargetUpper, GetWorld()->GetDeltaSeconds(), 8.0f);
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Foot IK — trace from foot bone positions down to ground
    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    // Left foot
    FVector LeftFootLoc = Mesh->GetSocketLocation(FName("foot_l"));
    FVector RightFootLoc = Mesh->GetSocketLocation(FName("foot_r"));

    FVector TraceStart_L = LeftFootLoc + FVector(0, 0, IKTraceDistance);
    FVector TraceEnd_L = LeftFootLoc - FVector(0, 0, IKTraceDistance);
    FVector TraceStart_R = RightFootLoc + FVector(0, 0, IKTraceDistance);
    FVector TraceEnd_R = RightFootLoc - FVector(0, 0, IKTraceDistance);

    FHitResult HitL, HitR;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    UWorld* World = GetWorld();
    if (!World) return;

    bool bHitL = World->LineTraceSingleByChannel(HitL, TraceStart_L, TraceEnd_L, ECC_Visibility, Params);
    bool bHitR = World->LineTraceSingleByChannel(HitR, TraceStart_R, TraceEnd_R, ECC_Visibility, Params);

    // Target offsets
    FVector TargetLeftOffset = FVector::ZeroVector;
    FVector TargetRightOffset = FVector::ZeroVector;

    if (bHitL)
    {
        float LeftDelta = HitL.ImpactPoint.Z - LeftFootLoc.Z;
        TargetLeftOffset = FVector(0, 0, FMath::Clamp(LeftDelta, -20.0f, 20.0f));
    }
    if (bHitR)
    {
        float RightDelta = HitR.ImpactPoint.Z - RightFootLoc.Z;
        TargetRightOffset = FVector(0, 0, FMath::Clamp(RightDelta, -20.0f, 20.0f));
    }

    // Smooth IK offsets
    LeftFootOffset = FMath::VInterpTo(LeftFootOffset, TargetLeftOffset, DeltaSeconds, IKSmoothingRate);
    RightFootOffset = FMath::VInterpTo(RightFootOffset, TargetRightOffset, DeltaSeconds, IKSmoothingRate);

    // Pelvis offset = lowest foot offset (to avoid stretching)
    float MinZ = FMath::Min(LeftFootOffset.Z, RightFootOffset.Z);
    FVector TargetPelvis = FVector(0, 0, FMath::Clamp(MinZ, -15.0f, 0.0f));
    PelvisOffset = FMath::VInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, IKSmoothingRate * 0.5f);

    // IK alpha — full IK only when grounded and not sprinting
    float TargetAlpha = (!bIsInAir && !bIsSprinting) ? 1.0f : 0.0f;
    LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, TargetAlpha, DeltaSeconds, IKSmoothingRate);
    RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, TargetAlpha, DeltaSeconds, IKSmoothingRate);
}

void UPlayerAnimInstance::SetStaminaNormalized(float Value)
{
    StaminaNormalized = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UPlayerAnimInstance::SetHealthNormalized(float Value)
{
    HealthNormalized = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UPlayerAnimInstance::SetFearLevel(float Value)
{
    FearLevel = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UPlayerAnimInstance::SetCombatStance(EAnim_CombatStance NewStance)
{
    CombatStance = NewStance;
}

EAnim_LocomotionState UPlayerAnimInstance::GetLocomotionState() const
{
    return LocomotionState;
}

bool UPlayerAnimInstance::IsPlayingMontage(UAnimMontage* Montage) const
{
    if (!Montage) return false;
    return Montage_IsPlaying(Montage);
}
