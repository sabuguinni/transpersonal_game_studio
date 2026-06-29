// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full implementation of UPlayerAnimInstance:
//   - Locomotion blend space inputs (speed, direction, lean)
//   - Survival state mirroring from TranspersonalCharacter
//   - Combat state tracking
//   - Two-bone foot IK with pelvis correction
//   - Aim offset (pitch/yaw) from control rotation

#include "PlayerAnimInstance.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion defaults
    Speed               = 0.f;
    SmoothedSpeed       = 0.f;
    Direction           = 0.f;
    bIsMoving           = false;
    bIsSprinting        = false;
    bIsCrouching        = false;
    bIsInAir            = false;
    LeanAmount          = 0.f;
    VerticalVelocity    = 0.f;

    // Survival defaults
    bIsSneaking         = false;
    bIsClimbing         = false;
    FearLevel           = 0.f;
    StaminaLevel        = 1.f;
    HungerLevel         = 0.f;
    bIsExhausted        = false;

    // Combat defaults
    bIsInCombat         = false;
    bIsAttacking        = false;
    bIsBlocking         = false;
    bIsThrowing         = false;
    bIsHit              = false;
    bIsDead             = false;
    WeaponTypeIndex     = 0;

    // Interaction defaults
    bIsGathering        = false;
    bIsCrafting         = false;
    bIsSwimming         = false;

    // Foot IK defaults
    LeftFootIKOffset    = FVector::ZeroVector;
    RightFootIKOffset   = FVector::ZeroVector;
    PelvisOffset        = 0.f;
    LeftFootIKAlpha     = 0.f;
    RightFootIKAlpha    = 0.f;

    // Aim offset defaults
    AimPitch            = 0.f;
    AimYaw              = 0.f;

    // Internal tuning
    SpeedSmoothingAlpha     = 8.f;
    LeanSmoothingAlpha      = 4.f;
    PreviousVelocity        = FVector::ZeroVector;
    FootIKTraceChannel      = ECC_Visibility;
    FootIKTraceHalfHeight   = 50.f;
    MaxPelvisCorrection     = 15.f;

    OwnerCharacter      = nullptr;
    MovementComponent   = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeInitializeAnimation
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter    = Cast<ATranspersonalCharacter>(Pawn);
    MovementComponent = OwnerCharacter
                        ? OwnerCharacter->GetCharacterMovement()
                        : nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeUpdateAnimation  (called every frame)
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent || DeltaSeconds <= 0.f)
        return;

    UpdateLocomotion(DeltaSeconds);
    UpdateSurvivalStates(DeltaSeconds);
    UpdateCombatStates(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
}

// ─────────────────────────────────────────────────────────────────────────────
// NativePostEvaluateAnimation
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::NativePostEvaluateAnimation()
{
    Super::NativePostEvaluateAnimation();
    // Reserved for post-evaluate bone transforms (e.g., hand IK corrections)
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateLocomotion
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    const FVector Velocity = OwnerCharacter->GetVelocity();
    Speed = Velocity.Size2D();

    // Smooth speed for blend space
    SmoothedSpeed = SmoothFloat(SmoothedSpeed, Speed, SpeedSmoothingAlpha, DeltaSeconds);

    // Moving threshold: 10 cm/s
    bIsMoving = Speed > 10.f;

    // Vertical velocity (positive = moving up)
    VerticalVelocity = Velocity.Z;

    // Direction angle relative to character facing
    if (bIsMoving)
    {
        const FRotator ActorRot = OwnerCharacter->GetActorRotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(
            Velocity.Rotation(), ActorRot).Yaw;
    }
    else
    {
        Direction = SmoothFloat(Direction, 0.f, 6.f, DeltaSeconds);
    }

    // Lean: angular acceleration of the character's yaw
    const FVector DeltaVelocity = Velocity - PreviousVelocity;
    const float   LateralAccel  = FVector::DotProduct(
        DeltaVelocity,
        OwnerCharacter->GetActorRightVector()) / FMath::Max(DeltaSeconds, 0.001f);

    const float TargetLean = FMath::Clamp(LateralAccel / 600.f, -1.f, 1.f);
    LeanAmount = SmoothFloat(LeanAmount, TargetLean, LeanSmoothingAlpha, DeltaSeconds);
    PreviousVelocity = Velocity;

    // Movement mode flags
    bIsInAir    = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsSwimming  = MovementComponent->IsSwimming();

    // Sprint: speed > 350 cm/s and not in air
    bIsSprinting = (Speed > 350.f) && !bIsInAir && !bIsCrouching;
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateSurvivalStates
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateSurvivalStates(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Mirror survival stats from TranspersonalCharacter
    // These properties are exposed as BlueprintReadOnly on the character
    // We use raw property access via the character pointer

    // Stamina — drives exhaustion posture and heavy breathing
    StaminaLevel = FMath::Clamp(OwnerCharacter->Stamina / 100.f, 0.f, 1.f);
    bIsExhausted = StaminaLevel < 0.1f;

    // Hunger — drives hunched idle posture when starving
    HungerLevel = FMath::Clamp(1.f - (OwnerCharacter->Hunger / 100.f), 0.f, 1.f);

    // Fear — drives shaking, wide-eyed head movement
    FearLevel = FMath::Clamp(OwnerCharacter->Fear / 100.f, 0.f, 1.f);

    // Sneak: crouching + slow speed
    bIsSneaking = bIsCrouching && Speed < 150.f;
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateCombatStates
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateCombatStates(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    bIsDead = OwnerCharacter->Health <= 0.f;

    // bIsInCombat, bIsAttacking, bIsBlocking, bIsThrowing, bIsHit,
    // WeaponTypeIndex are set externally by the combat system via
    // direct property assignment (no getter needed — Blueprint-readable).
    // The combat system calls SetBoolProperty / SetIntProperty on this
    // AnimInstance when state changes occur.
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateAimOffset
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    const AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    const FRotator ControlRot  = Controller->GetControlRotation();
    const FRotator ActorRot    = OwnerCharacter->GetActorRotation();
    const FRotator DeltaRot    = UKismetMathLibrary::NormalizedDeltaRotator(
        ControlRot, ActorRot);

    AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.f, 90.f);
    AimYaw   = FMath::Clamp(DeltaRot.Yaw,   -90.f, 90.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateFootIK
// Performs two line traces (left/right foot sockets) and computes:
//   - Per-foot vertical offset to match terrain
//   - Pelvis offset to prevent leg over-extension
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Disable foot IK while airborne or swimming
    if (bIsInAir || bIsSwimming)
    {
        LeftFootIKAlpha  = SmoothFloat(LeftFootIKAlpha,  0.f, 10.f, DeltaSeconds);
        RightFootIKAlpha = SmoothFloat(RightFootIKAlpha, 0.f, 10.f, DeltaSeconds);
        PelvisOffset     = SmoothFloat(PelvisOffset,     0.f, 10.f, DeltaSeconds);
        return;
    }

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    const FName LeftFootSocket  = TEXT("foot_l");
    const FName RightFootSocket = TEXT("foot_r");

    auto TraceFootSocket = [&](const FName& SocketName, FVector& OutOffset, float& OutAlpha) -> float
    {
        const FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
        const FVector TraceStart = SocketLoc + FVector(0.f, 0.f, FootIKTraceHalfHeight);
        const FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, FootIKTraceHalfHeight * 2.f);

        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, FootIKTraceChannel, Params))
        {
            const float HitZ     = HitResult.ImpactPoint.Z;
            const float SocketZ  = SocketLoc.Z;
            const float TargetZ  = HitZ - SocketZ;

            OutOffset = FVector(0.f, 0.f, SmoothFloat(OutOffset.Z, TargetZ, 10.f, DeltaSeconds));
            OutAlpha  = SmoothFloat(OutAlpha, 1.f, 10.f, DeltaSeconds);
            return TargetZ;
        }
        else
        {
            OutOffset = FVector(0.f, 0.f, SmoothFloat(OutOffset.Z, 0.f, 10.f, DeltaSeconds));
            OutAlpha  = SmoothFloat(OutAlpha, 0.f, 10.f, DeltaSeconds);
            return 0.f;
        }
    };

    float LeftZ  = TraceFootSocket(LeftFootSocket,  LeftFootIKOffset,  LeftFootIKAlpha);
    float RightZ = TraceFootSocket(RightFootSocket, RightFootIKOffset, RightFootIKAlpha);

    // Pelvis: move down by the larger of the two foot corrections
    // so the higher foot stays planted without leg over-extension
    const float TargetPelvis = FMath::Min(LeftZ, RightZ);
    PelvisOffset = SmoothFloat(
        PelvisOffset,
        FMath::Clamp(TargetPelvis, -MaxPelvisCorrection, 0.f),
        10.f,
        DeltaSeconds);
}

// ─────────────────────────────────────────────────────────────────────────────
// Blueprint callable utilities
// ─────────────────────────────────────────────────────────────────────────────

void UPlayerAnimInstance::PlayMontageByName(const FName& MontageName, float PlayRate)
{
    // Montage asset lookup is handled by the Animation Blueprint.
    // This stub is intentionally lightweight — the Blueprint overrides
    // this function to call Montage_Play with the correct asset reference.
    UE_LOG(LogTemp, Log, TEXT("UPlayerAnimInstance::PlayMontageByName — %s (rate %.2f)"),
        *MontageName.ToString(), PlayRate);
}

void UPlayerAnimInstance::StopActiveMontage(float BlendOutTime)
{
    Montage_StopGroupByName(BlendOutTime, NAME_None);
}

bool UPlayerAnimInstance::IsAnyMontageActive() const
{
    return IsAnyMontagePlaying();
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal helpers
// ─────────────────────────────────────────────────────────────────────────────

float UPlayerAnimInstance::SmoothFloat(float Current, float Target, float Alpha, float DeltaSeconds) const
{
    return FMath::FInterpTo(Current, Target, DeltaSeconds, Alpha);
}
