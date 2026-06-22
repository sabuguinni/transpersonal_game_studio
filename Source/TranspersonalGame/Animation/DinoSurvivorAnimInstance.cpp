// DinoSurvivorAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full AnimInstance implementation: locomotion state machine, bilateral foot IK,
// survival posture blending, lean, aim offset, jump/land triggers.

#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / NativeInitializeAnimation
// ─────────────────────────────────────────────────────────────────────────────

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    // Locomotion defaults
    Speed              = 0.f;
    Direction          = 0.f;
    bIsInAir           = false;
    bIsCrouching       = false;
    bIsSprinting       = false;
    bIsMoving          = false;
    bShouldMove        = false;
    bJustLanded        = false;
    LandVelocityZ      = 0.f;
    AimPitch           = 0.f;
    AimYaw             = 0.f;
    LeanAngle          = 0.f;
    PostureBlend       = 0.f;

    // Foot IK defaults
    LeftFootOffset     = FVector::ZeroVector;
    RightFootOffset    = FVector::ZeroVector;
    PelvisOffset       = FVector::ZeroVector;
    LeftFootRotation   = FRotator::ZeroRotator;
    RightFootRotation  = FRotator::ZeroRotator;
    IKTraceDistance    = 50.f;
    IKInterpSpeed      = 15.f;

    // Survival posture
    SurvivalPosture    = EAnim_SurvivalPosture::Upright;
    StaminaRatio       = 1.f;
    HealthRatio        = 1.f;
    FearLevel          = 0.f;

    // Internal
    OwnerCharacter     = nullptr;
    OwnerMovement      = nullptr;
    PreviousVelocityZ  = 0.f;
    bWasInAir          = false;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
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

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !OwnerMovement || DeltaSeconds <= 0.f) return;

    UpdateLocomotionState(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
    UpdateLean(DeltaSeconds);
    UpdateSurvivalPosture(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
}

// ─────────────────────────────────────────────────────────────────────────────
// Locomotion
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
    const FVector Velocity    = OwnerMovement->Velocity;
    const FVector VelocityXY  = FVector(Velocity.X, Velocity.Y, 0.f);

    Speed      = VelocityXY.Size();
    bIsInAir   = OwnerMovement->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsMoving  = Speed > 3.f;

    // Determine sprint: speed > walk threshold (300 cm/s default walk)
    const float WalkSpeed  = OwnerMovement->MaxWalkSpeed;
    bIsSprinting = (Speed > WalkSpeed * 0.85f) && !bIsCrouching && !bIsInAir;

    // Direction relative to actor forward
    if (bIsMoving)
    {
        const FRotator ActorRot = OwnerCharacter->GetActorRotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(
            VelocityXY.Rotation(), ActorRot).Yaw;
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.f, DeltaSeconds, 8.f);
    }

    bShouldMove = bIsMoving && OwnerMovement->GetCurrentAcceleration().SizeSquared() > 0.f;

    // Land detection
    const bool bCurrentlyInAir = bIsInAir;
    if (bWasInAir && !bCurrentlyInAir)
    {
        bJustLanded    = true;
        LandVelocityZ  = FMath::Abs(PreviousVelocityZ);
    }
    else
    {
        bJustLanded = false;
    }

    bWasInAir        = bCurrentlyInAir;
    PreviousVelocityZ = Velocity.Z;
}

// ─────────────────────────────────────────────────────────────────────────────
// Aim Offset
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    const FRotator ActorRot    = OwnerCharacter->GetActorRotation();
    const FRotator ControlRot  = OwnerCharacter->GetControlRotation();
    const FRotator DeltaRot    = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.f, 90.f);
    AimYaw   = FMath::ClampAngle(DeltaRot.Yaw,   -90.f, 90.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Lean
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    const FVector AngularVelocity = OwnerMovement->GetLastUpdateVelocity();
    const FVector LocalVelocity   = OwnerCharacter->GetActorTransform().InverseTransformVector(AngularVelocity);

    const float TargetLean = FMath::Clamp(LocalVelocity.Y * 0.02f, -45.f, 45.f);
    LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 6.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival Posture
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalPosture(float DeltaSeconds)
{
    // Determine posture priority: Injured > Exhausted > Fleeing > Cautious > Upright
    EAnim_SurvivalPosture Target = EAnim_SurvivalPosture::Upright;

    if (HealthRatio < 0.25f)
    {
        Target = EAnim_SurvivalPosture::Injured;
    }
    else if (StaminaRatio < 0.15f)
    {
        Target = EAnim_SurvivalPosture::Exhausted;
    }
    else if (FearLevel > 0.75f)
    {
        Target = EAnim_SurvivalPosture::Fleeing;
    }
    else if (FearLevel > 0.4f)
    {
        Target = EAnim_SurvivalPosture::Cautious;
    }

    SurvivalPosture = Target;

    // PostureBlend: 0=Upright, 1=Injured (drives additive layer weight)
    float TargetBlend = 0.f;
    switch (SurvivalPosture)
    {
        case EAnim_SurvivalPosture::Cautious:  TargetBlend = 0.25f; break;
        case EAnim_SurvivalPosture::Exhausted: TargetBlend = 0.55f; break;
        case EAnim_SurvivalPosture::Fleeing:   TargetBlend = 0.35f; break;
        case EAnim_SurvivalPosture::Injured:   TargetBlend = 1.0f;  break;
        default:                               TargetBlend = 0.f;   break;
    }

    PostureBlend = FMath::FInterpTo(PostureBlend, TargetBlend, DeltaSeconds, 3.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Foot IK — bilateral terrain-adaptive traces
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir) 
    {
        // Smoothly reset IK offsets when airborne
        LeftFootOffset  = FMath::VInterpTo(LeftFootOffset,  FVector::ZeroVector, DeltaSeconds, IKInterpSpeed);
        RightFootOffset = FMath::VInterpTo(RightFootOffset, FVector::ZeroVector, DeltaSeconds, IKInterpSpeed);
        PelvisOffset    = FMath::VInterpTo(PelvisOffset,    FVector::ZeroVector, DeltaSeconds, IKInterpSpeed);
        return;
    }

    const FVector LeftTarget  = TraceFootIK(FName("foot_l"), DeltaSeconds);
    const FVector RightTarget = TraceFootIK(FName("foot_r"), DeltaSeconds);

    LeftFootOffset  = FMath::VInterpTo(LeftFootOffset,  LeftTarget,  DeltaSeconds, IKInterpSpeed);
    RightFootOffset = FMath::VInterpTo(RightFootOffset, RightTarget, DeltaSeconds, IKInterpSpeed);

    // Pelvis drops to accommodate the lower foot
    const float LowestFoot = FMath::Min(LeftFootOffset.Z, RightFootOffset.Z);
    const FVector TargetPelvis = FVector(0.f, 0.f, LowestFoot);
    PelvisOffset = FMath::VInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, IKInterpSpeed * 0.5f);
}

FVector UDinoSurvivorAnimInstance::TraceFootIK(FName SocketName, float DeltaSeconds)
{
    if (!OwnerCharacter) return FVector::ZeroVector;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return FVector::ZeroVector;

    const FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    const FVector TraceStart     = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z + IKTraceDistance);
    const FVector TraceEnd       = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z - IKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (bHit)
    {
        const float HitZ        = HitResult.ImpactPoint.Z;
        const float CapsuleHalf = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        const float ActorZ      = OwnerCharacter->GetActorLocation().Z - CapsuleHalf;
        const float OffsetZ     = HitZ - ActorZ;

        // Foot rotation from surface normal
        const FVector Normal    = HitResult.ImpactNormal;
        const FRotator FootRot  = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
            0.f,
            FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z))
        );

        if (SocketName == FName("foot_l"))  LeftFootRotation  = FootRot;
        if (SocketName == FName("foot_r"))  RightFootRotation = FootRot;

        return FVector(0.f, 0.f, OffsetZ);
    }

    return FVector::ZeroVector;
}

// ─────────────────────────────────────────────────────────────────────────────
// Blueprint-callable setters (called from TranspersonalCharacter)
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::SetSurvivalStats(float Health, float Stamina, float Fear)
{
    HealthRatio  = FMath::Clamp(Health,  0.f, 1.f);
    StaminaRatio = FMath::Clamp(Stamina, 0.f, 1.f);
    FearLevel    = FMath::Clamp(Fear,    0.f, 1.f);
}
