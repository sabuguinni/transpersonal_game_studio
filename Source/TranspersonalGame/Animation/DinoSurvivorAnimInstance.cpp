#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    // Locomotion defaults
    LocomotionState     = EAnim_LocomotionState::Idle;
    CombatStance        = EAnim_CombatStance::Unarmed;
    Speed               = 0.0f;
    Direction           = 0.0f;
    LeanAngle           = 0.0f;
    AimPitch            = 0.0f;
    AimYaw              = 0.0f;
    StaminaNormalized   = 1.0f;
    HealthNormalized    = 1.0f;
    bIsInAir            = false;
    bIsCrouching        = false;
    bIsSprinting        = false;
    bIsExhausted        = false;
    bIsInjured          = false;
    bIsAttacking        = false;
    bIsBlocking         = false;

    // Speed thresholds (cm/s)
    WalkSpeedThreshold   = 10.0f;
    RunSpeedThreshold    = 300.0f;
    SprintSpeedThreshold = 550.0f;

    // Foot IK defaults
    FootIKTraceDistance = 80.0f;
    FootIKInterpSpeed   = 15.0f;

    OwnerCharacter    = nullptr;
    MovementComponent = nullptr;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter = Cast<ACharacter>(Pawn);
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent) return;

    UpdateLocomotionState();
    UpdateFootIK(DeltaSeconds);
    UpdateAimOffset();
    UpdateSurvivalState();
}

// ─────────────────────────────────────────────────────────────
// Locomotion state machine
// ─────────────────────────────────────────────────────────────
void UDinoSurvivorAnimInstance::UpdateLocomotionState()
{
    if (!OwnerCharacter || !MovementComponent) return;

    const FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();

    // Direction relative to character facing
    const FRotator CharRotation = OwnerCharacter->GetActorRotation();
    Direction = UKismetMathLibrary::NormalizedDeltaRotator(
        UKismetMathLibrary::MakeRotFromX(Velocity.GetSafeNormal()),
        CharRotation
    ).Yaw;

    bIsInAir    = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Sprint: detect via max walk speed being elevated
    bIsSprinting = (Speed > SprintSpeedThreshold);

    // Lean — simple lateral acceleration-based lean
    const FVector Accel = MovementComponent->GetCurrentAcceleration();
    const FVector LocalAccel = CharRotation.UnrotateVector(Accel);
    LeanAngle = FMath::FInterpTo(LeanAngle, LocalAccel.Y * 0.02f, GetWorld()->GetDeltaSeconds(), 5.0f);

    // State transitions
    if (bIsInAir)
    {
        LocomotionState = (Velocity.Z > 0.0f)
            ? EAnim_LocomotionState::Jump
            : EAnim_LocomotionState::Fall;
    }
    else if (bIsCrouching)
    {
        LocomotionState = (Speed > WalkSpeedThreshold)
            ? EAnim_LocomotionState::CrouchWalk
            : EAnim_LocomotionState::Crouch;
    }
    else if (Speed > SprintSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed > RunSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else if (Speed > WalkSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
}

// ─────────────────────────────────────────────────────────────
// Foot IK — terrain-adaptive foot placement
// ─────────────────────────────────────────────────────────────
void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir) return;

    FVector LeftLoc  = FVector::ZeroVector;
    FVector RightLoc = FVector::ZeroVector;
    FRotator LeftRot  = FRotator::ZeroRotator;
    FRotator RightRot = FRotator::ZeroRotator;

    const bool bLeftHit  = TraceFootToGround(FName("foot_l"), LeftLoc, LeftRot);
    const bool bRightHit = TraceFootToGround(FName("foot_r"), RightLoc, RightRot);

    // Smooth interpolation
    const float InterpSpeed = FootIKInterpSpeed;
    FootIKData.LeftFootLocation = FMath::VInterpTo(
        FootIKData.LeftFootLocation, LeftLoc, DeltaSeconds, InterpSpeed);
    FootIKData.RightFootLocation = FMath::VInterpTo(
        FootIKData.RightFootLocation, RightLoc, DeltaSeconds, InterpSpeed);
    FootIKData.LeftFootRotation = FMath::RInterpTo(
        FootIKData.LeftFootRotation, LeftRot, DeltaSeconds, InterpSpeed);
    FootIKData.RightFootRotation = FMath::RInterpTo(
        FootIKData.RightFootRotation, RightRot, DeltaSeconds, InterpSpeed);

    FootIKData.LeftFootAlpha  = bLeftHit  ? 1.0f : 0.0f;
    FootIKData.RightFootAlpha = bRightHit ? 1.0f : 0.0f;

    // Pelvis offset — lower pelvis to accommodate the lower foot
    const float LeftZ  = bLeftHit  ? LeftLoc.Z  : 0.0f;
    const float RightZ = bRightHit ? RightLoc.Z : 0.0f;
    const float MinZ   = FMath::Min(LeftZ, RightZ);
    FootIKData.PelvisOffset = FMath::FInterpTo(
        FootIKData.PelvisOffset, MinZ, DeltaSeconds, InterpSpeed);
}

bool UDinoSurvivorAnimInstance::TraceFootToGround(
    const FName& FootSocketName,
    FVector& OutLocation,
    FRotator& OutRotation)
{
    if (!OwnerCharacter) return false;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return false;

    const FVector FootWorldPos = Mesh->GetSocketLocation(FootSocketName);
    const FVector TraceStart   = FootWorldPos + FVector(0.0f, 0.0f, FootIKTraceDistance);
    const FVector TraceEnd     = FootWorldPos - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (bHit)
    {
        // Offset relative to character root
        const FVector CharBase = OwnerCharacter->GetActorLocation();
        OutLocation = FVector(0.0f, 0.0f, HitResult.Location.Z - CharBase.Z);

        // Foot rotation from surface normal
        const FVector Normal = HitResult.Normal;
        OutRotation = FRotator(
            FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
            0.0f,
            -FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z))
        );
    }

    return bHit;
}

// ─────────────────────────────────────────────────────────────
// Aim offset — pitch/yaw for upper-body aiming
// ─────────────────────────────────────────────────────────────
void UDinoSurvivorAnimInstance::UpdateAimOffset()
{
    if (!OwnerCharacter) return;

    const AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    const FRotator ControlRot = Controller->GetControlRotation();
    const FRotator ActorRot   = OwnerCharacter->GetActorRotation();
    const FRotator Delta      = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::ClampAngle(Delta.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::ClampAngle(Delta.Yaw,   -90.0f, 90.0f);
}

// ─────────────────────────────────────────────────────────────
// Survival state — exhaustion/injury affect animation
// ─────────────────────────────────────────────────────────────
void UDinoSurvivorAnimInstance::UpdateSurvivalState()
{
    // Survival stats are read from the character if it exposes them.
    // Using a safe property-access pattern — no hard dependency on custom class.
    // If TranspersonalCharacter exposes stamina/health as floats, read them here.
    // For now, derive from speed/movement as proxy:

    // Exhaustion: sprint speed sustained drops stamina proxy
    if (bIsSprinting)
    {
        StaminaNormalized = FMath::FInterpTo(StaminaNormalized, 0.0f,
            GetWorld()->GetDeltaSeconds(), 0.05f);
    }
    else
    {
        StaminaNormalized = FMath::FInterpTo(StaminaNormalized, 1.0f,
            GetWorld()->GetDeltaSeconds(), 0.1f);
    }

    bIsExhausted = (StaminaNormalized < 0.15f);
    bIsInjured   = (HealthNormalized  < 0.35f);

    // When exhausted, cap locomotion to Walk
    if (bIsExhausted && LocomotionState == EAnim_LocomotionState::Sprint)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
}
