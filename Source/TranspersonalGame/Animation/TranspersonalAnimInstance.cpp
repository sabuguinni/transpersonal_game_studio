#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    Speed = 0.f;
    Direction = 0.f;
    LeanAngle = 0.f;
    VerticalVelocity = 0.f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsSprinting = false;
    bIsMoving = false;
    LocomotionState = EAnim_LocomotionState::Idle;
    StanceType = EAnim_StanceType::Standing;

    // Foot IK defaults
    LeftFootEffectorLocation = FVector::ZeroVector;
    RightFootEffectorLocation = FVector::ZeroVector;
    LeftFootRotation = FRotator::ZeroRotator;
    RightFootRotation = FRotator::ZeroRotator;
    IKAlpha = 0.f;
    IKTraceDistance = 50.f;

    // Survival defaults
    HealthNormalized = 1.f;
    StaminaNormalized = 1.f;
    bIsExhausted = false;
    bIsWounded = false;
    bIsDead = false;

    // Aim offset defaults
    AimPitch = 0.f;
    AimYaw = 0.f;

    // Internal
    OwnerCharacter = nullptr;
    PreviousVelocity = FVector::ZeroVector;
    LeanAlpha = 0.f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (!OwnerCharacter) return;
    }

    UpdateLocomotion(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalState(DeltaSeconds);
    UpdateAimOffset(DeltaSeconds);
}

// ─────────────────────────────────────────────────────────────────────────────
// Locomotion
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
    if (!MoveComp) return;

    const FVector Velocity = OwnerCharacter->GetVelocity();
    Speed = Velocity.Size2D();
    VerticalVelocity = Velocity.Z;
    bIsInAir = MoveComp->IsFalling();
    bIsCrouching = MoveComp->IsCrouching();
    bIsMoving = Speed > 3.f;

    // Direction (strafe angle relative to actor forward)
    if (bIsMoving)
    {
        const FRotator ActorRot = OwnerCharacter->GetActorRotation();
        const FRotator VelocityRot = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.f;
    }

    // Sprint detection — speed above walk threshold (300 cm/s default walk)
    bIsSprinting = (Speed > 350.f) && !bIsInAir && !bIsCrouching;

    // Lean — lateral acceleration smoothed
    const FVector Accel = (Velocity - PreviousVelocity) / FMath::Max(DeltaSeconds, KINDA_SMALL_NUMBER);
    const float LateralAccel = FVector::DotProduct(Accel, OwnerCharacter->GetActorRightVector());
    LeanAlpha = FMath::FInterpTo(LeanAlpha, LateralAccel, DeltaSeconds, 4.f);
    LeanAngle = FMath::Clamp(LeanAlpha * 0.02f, -15.f, 15.f);
    PreviousVelocity = Velocity;

    // Stance
    if (bIsCrouching)
        StanceType = EAnim_StanceType::Crouched;
    else
        StanceType = EAnim_StanceType::Standing;

    LocomotionState = DetermineLocomotionState();
}

EAnim_LocomotionState UTranspersonalAnimInstance::DetermineLocomotionState() const
{
    if (bIsDead)        return EAnim_LocomotionState::Dead;
    if (bIsInAir)
    {
        return (VerticalVelocity > 0.f)
            ? EAnim_LocomotionState::Jumping
            : EAnim_LocomotionState::Falling;
    }
    if (bIsCrouching)   return EAnim_LocomotionState::Crouching;
    if (bIsSprinting)   return EAnim_LocomotionState::Sprinting;
    if (bIsMoving)      return EAnim_LocomotionState::Walking;
    return EAnim_LocomotionState::Idle;
}

// ─────────────────────────────────────────────────────────────────────────────
// Foot IK — line traces to adapt feet to uneven terrain
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir)
    {
        IKAlpha = FMath::FInterpTo(IKAlpha, 0.f, DeltaSeconds, 10.f);
        return;
    }

    IKAlpha = FMath::FInterpTo(IKAlpha, 1.f, DeltaSeconds, 10.f);

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    const FName LeftFootSocket  = TEXT("foot_l");
    const FName RightFootSocket = TEXT("foot_r");

    auto TraceFootIK = [&](FName SocketName, FVector& OutEffector, FRotator& OutRotation)
    {
        USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
        if (!Mesh) return;

        const FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
        const FVector TraceStart = SocketLoc + FVector(0.f, 0.f, IKTraceDistance);
        const FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, IKTraceDistance);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            // Offset from actor root to hit point
            const float ZOffset = Hit.Location.Z - OwnerCharacter->GetActorLocation().Z;
            OutEffector = FVector(0.f, 0.f, ZOffset);

            // Surface normal to foot rotation
            const FVector Normal = Hit.Normal;
            OutRotation = FRotator(
                FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
                0.f,
                -FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z))
            );
        }
        else
        {
            OutEffector = FVector::ZeroVector;
            OutRotation = FRotator::ZeroRotator;
        }
    };

    TraceFootIK(LeftFootSocket,  LeftFootEffectorLocation,  LeftFootRotation);
    TraceFootIK(RightFootSocket, RightFootEffectorLocation, RightFootRotation);
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival State
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    // Try to read survival stats from the character via interface or cast
    // For now, use a safe property-based approach with defaults
    // Full integration happens when TranspersonalCharacter exposes these via BlueprintReadOnly

    // Wounded posture when health below 40%
    bIsWounded = (HealthNormalized < 0.4f);

    // Exhausted when stamina below 15%
    bIsExhausted = (StaminaNormalized < 0.15f);

    // Dead state locks all other locomotion
    bIsDead = (HealthNormalized <= 0.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Aim Offset
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    const FRotator ControlRot = Controller->GetControlRotation();
    const FRotator ActorRot   = OwnerCharacter->GetActorRotation();
    const FRotator Delta      = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);
    AimYaw   = FMath::Clamp(Delta.Yaw,   -90.f, 90.f);
}
