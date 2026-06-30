#include "PlayerAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UPlayerAnimInstance::UPlayerAnimInstance()
    : LocomotionState(EAnim_LocomotionState::Idle)
    , WeaponState(EAnim_WeaponState::Unarmed)
    , Speed(0.f)
    , Direction(0.f)
    , LeanAngle(0.f)
    , bIsInAir(false)
    , bIsCrouching(false)
    , bIsSprinting(false)
    , VerticalVelocity(0.f)
    , bIsAiming(false)
    , AimPitch(0.f)
    , AimYaw(0.f)
    , StaminaNormalized(1.f)
    , HealthNormalized(1.f)
    , bIsExhausted(false)
    , bIsInjured(false)
    , LeftFootIKLocation(FVector::ZeroVector)
    , RightFootIKLocation(FVector::ZeroVector)
    , FootIKAlpha(1.f)
    , WalkSpeedThreshold(10.f)
    , RunSpeedThreshold(300.f)
    , SprintSpeedThreshold(500.f)
    , IKTraceDistance(60.f)
    , OwnerCharacter(nullptr)
{
}

// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        OwnerCharacter = Cast<ACharacter>(Pawn);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        APawn* Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            OwnerCharacter = Cast<ACharacter>(Pawn);
        }
        if (!OwnerCharacter) return;
    }

    UpdateLocomotionState();
    UpdateWeaponState();
    UpdateSurvivalStats();
    UpdateFootIK();
}

// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateLocomotionState()
{
    UCharacterMovementComponent* MovComp =
        OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    const FVector Velocity = MovComp->Velocity;
    Speed = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;

    bIsInAir    = MovComp->IsFalling();
    bIsCrouching = MovComp->IsCrouching();
    bIsSprinting = (Speed >= SprintSpeedThreshold);

    // Direction (strafe angle relative to actor forward)
    const FRotator ActorRot = OwnerCharacter->GetActorRotation();
    Direction = UKismetMathLibrary::NormalizedDeltaRotator(
        UKismetMathLibrary::MakeRotFromX(Velocity.GetSafeNormal()),
        ActorRot).Yaw;

    // Lean — simple approximation from lateral velocity change
    LeanAngle = FMath::Clamp(Direction * 0.05f, -15.f, 15.f);

    // State machine
    if (bIsInAir)
    {
        LocomotionState = (VerticalVelocity < -100.f)
            ? EAnim_LocomotionState::InAir
            : EAnim_LocomotionState::InAir;
    }
    else if (bIsCrouching)
    {
        LocomotionState = (Speed > WalkSpeedThreshold)
            ? EAnim_LocomotionState::Sneak
            : EAnim_LocomotionState::Crouch;
    }
    else if (Speed >= SprintSpeedThreshold)
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
void UPlayerAnimInstance::UpdateWeaponState()
{
    // Default: Unarmed. Extended by TranspersonalCharacter inventory system.
    // WeaponState is set externally by the character's weapon component.
    // Here we just ensure AimPitch/Yaw are updated when aiming.
    if (bIsAiming)
    {
        const FRotator ControlRot =
            OwnerCharacter->GetControlRotation();
        const FRotator ActorRot =
            OwnerCharacter->GetActorRotation();
        const FRotator Delta =
            UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

        AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);
        AimYaw   = FMath::Clamp(Delta.Yaw,  -90.f, 90.f);
    }
    else
    {
        AimPitch = FMath::FInterpTo(AimPitch, 0.f,
            GetWorld()->GetDeltaSeconds(), 5.f);
        AimYaw   = FMath::FInterpTo(AimYaw,   0.f,
            GetWorld()->GetDeltaSeconds(), 5.f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateSurvivalStats()
{
    // Survival stats are driven by TranspersonalCharacter properties.
    // Exhaustion and injury affect locomotion blend weights.
    bIsExhausted = (StaminaNormalized < 0.15f);
    bIsInjured   = (HealthNormalized  < 0.30f);

    // When exhausted, cap effective speed for animation blending
    if (bIsExhausted && Speed > RunSpeedThreshold)
    {
        Speed = RunSpeedThreshold;
        bIsSprinting = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter || bIsInAir)
    {
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, 0.f,
            GetWorld()->GetDeltaSeconds(), 10.f);
        return;
    }

    FootIKAlpha = FMath::FInterpTo(FootIKAlpha, 1.f,
        GetWorld()->GetDeltaSeconds(), 10.f);

    LeftFootIKLocation  = TraceFootIK(FName("foot_l"));
    RightFootIKLocation = TraceFootIK(FName("foot_r"));
}

// ─────────────────────────────────────────────────────────────────────────────
FVector UPlayerAnimInstance::TraceFootIK(const FName& FootSocketName) const
{
    if (!OwnerCharacter) return FVector::ZeroVector;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return FVector::ZeroVector;

    const FVector SocketLoc = Mesh->GetSocketLocation(FootSocketName);
    const FVector TraceStart = SocketLoc + FVector(0.f, 0.f, IKTraceDistance);
    const FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, IKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams);

    return bHit ? HitResult.ImpactPoint : SocketLoc;
}
