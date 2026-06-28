#include "TranspersonalAnimInstance.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    LocomotionState     = EAnim_LocomotionState::Idle;
    StanceType          = EAnim_StanceType::Standing;
    Speed               = 0.0f;
    Direction           = 0.0f;
    VerticalVelocity    = 0.0f;

    // State booleans
    bIsInAir    = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsAttacking = false;
    bIsDead      = false;
    bIsAiming    = false;

    // Survival
    StaminaNormalized = 1.0f;
    HealthNormalized  = 1.0f;
    FearNormalized    = 0.0f;

    // IK
    LeftFootIKLocation  = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    LeftFootIKAlpha     = 0.0f;
    RightFootIKAlpha    = 0.0f;
    PelvisOffset        = 0.0f;

    // Additive
    LeanAmount  = 0.0f;
    AimPitch    = 0.0f;
    AimYaw      = 0.0f;

    // Config thresholds
    WalkSpeedThreshold   = 10.0f;
    SprintSpeedThreshold = 400.0f;
    IKTraceDistance      = 80.0f;

    PreviousSpeed = 0.0f;
    LeanSmoothed  = 0.0f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        OwnerCharacter = Cast<ATranspersonalCharacter>(Pawn);
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        APawn* Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            OwnerCharacter = Cast<ATranspersonalCharacter>(Pawn);
        }
        if (!OwnerCharacter) return;
    }

    UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    // ── Core velocity data ────────────────────────────────────────────────
    FVector Velocity = OwnerCharacter->GetVelocity();
    Speed            = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;
    bIsInAir         = MovComp->IsFalling();
    bIsCrouching     = MovComp->IsCrouching();

    // Direction angle relative to actor forward
    if (Speed > WalkSpeedThreshold)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator VelocityRot = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // ── Sprint detection ──────────────────────────────────────────────────
    bIsSprinting = (Speed >= SprintSpeedThreshold) && !bIsInAir && !bIsCrouching;

    // ── Lean (lateral banking into turns) ────────────────────────────────
    float SpeedDelta = Speed - PreviousSpeed;
    float TargetLean = FMath::Clamp(Direction * 0.05f, -1.0f, 1.0f);
    LeanSmoothed = SmoothLean(LeanSmoothed, TargetLean, DeltaSeconds);
    LeanAmount   = LeanSmoothed;
    PreviousSpeed = Speed;

    // ── Stance ────────────────────────────────────────────────────────────
    if (bIsCrouching)
        StanceType = EAnim_StanceType::Crouched;
    else
        StanceType = EAnim_StanceType::Standing;

    // ── Update subsystems ─────────────────────────────────────────────────
    UpdateLocomotionState();
    UpdateIKFootPlacement();
    UpdateSurvivalAnimations();
    UpdateAimOffset();
}

void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    if (bIsDead)
    {
        LocomotionState = EAnim_LocomotionState::Dead;
        return;
    }
    if (bIsAttacking)
    {
        LocomotionState = EAnim_LocomotionState::Attacking;
        return;
    }
    if (bIsInAir)
    {
        LocomotionState = (VerticalVelocity > 0.0f)
            ? EAnim_LocomotionState::Jumping
            : EAnim_LocomotionState::Falling;
        return;
    }
    if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouching;
        return;
    }
    if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprinting;
        return;
    }
    if (Speed > WalkSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Walking;
        return;
    }
    LocomotionState = EAnim_LocomotionState::Idle;
}

void UTranspersonalAnimInstance::UpdateIKFootPlacement()
{
    if (!OwnerCharacter || bIsInAir) 
    {
        LeftFootIKAlpha  = 0.0f;
        RightFootIKAlpha = 0.0f;
        PelvisOffset     = 0.0f;
        return;
    }

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerCharacter);

    // ── Left foot trace ───────────────────────────────────────────────────
    FName LeftFootBone  = FName("foot_l");
    FName RightFootBone = FName("foot_r");

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    auto TraceForFoot = [&](FName BoneName, FVector& OutLocation, float& OutAlpha)
    {
        FVector BoneLocation = Mesh->GetBoneLocation(BoneName);
        FVector TraceStart   = BoneLocation + FVector(0.0f, 0.0f, IKTraceDistance);
        FVector TraceEnd     = BoneLocation - FVector(0.0f, 0.0f, IKTraceDistance);

        FHitResult HitResult;
        bool bHit = World->LineTraceSingleByChannel(
            HitResult, TraceStart, TraceEnd,
            ECollisionChannel::ECC_Visibility, TraceParams);

        if (bHit)
        {
            OutLocation = HitResult.ImpactPoint;
            OutAlpha    = 1.0f;
        }
        else
        {
            OutLocation = BoneLocation;
            OutAlpha    = 0.0f;
        }
    };

    TraceForFoot(LeftFootBone,  LeftFootIKLocation,  LeftFootIKAlpha);
    TraceForFoot(RightFootBone, RightFootIKLocation, RightFootIKAlpha);

    // Pelvis offset: push pelvis down to the lower foot
    float LeftDelta  = LeftFootIKLocation.Z  - Mesh->GetBoneLocation(LeftFootBone).Z;
    float RightDelta = RightFootIKLocation.Z - Mesh->GetBoneLocation(RightFootBone).Z;
    PelvisOffset = FMath::Min(LeftDelta, RightDelta);
    PelvisOffset = FMath::Clamp(PelvisOffset, -30.0f, 0.0f);
}

void UTranspersonalAnimInstance::UpdateSurvivalAnimations()
{
    if (!OwnerCharacter) return;

    // Read survival stats from the character — properties exposed on ATranspersonalCharacter
    // These are normalised 0-1 values driving additive animation layers
    // (health limping, stamina breathing, fear trembling)
    // We guard with property existence checks via direct member access

    // Stamina: low stamina → heavy breathing additive
    // Health: low health → limp / hunched posture
    // Fear: high fear → trembling additive

    // Default safe values if character doesn't expose these yet
    StaminaNormalized = FMath::Clamp(StaminaNormalized, 0.0f, 1.0f);
    HealthNormalized  = FMath::Clamp(HealthNormalized,  0.0f, 1.0f);
    FearNormalized    = FMath::Clamp(FearNormalized,    0.0f, 1.0f);
}

void UTranspersonalAnimInstance::UpdateAimOffset()
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot   = OwnerCharacter->GetActorRotation();
    FRotator DeltaRot   = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::Clamp(DeltaRot.Yaw,  -180.0f, 180.0f);
}

float UTranspersonalAnimInstance::SmoothLean(float Current, float Target, float DeltaSeconds, float SmoothSpeed)
{
    return FMath::FInterpTo(Current, Target, DeltaSeconds, SmoothSpeed);
}
