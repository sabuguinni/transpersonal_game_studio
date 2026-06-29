#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
UPlayerAnimInstance::UPlayerAnimInstance()
    : Speed(0.f)
    , Direction(0.f)
    , SmoothedSpeed(0.f)
    , bIsInAir(false)
    , bIsCrouching(false)
    , bIsSprinting(false)
    , bIsAccelerating(false)
    , bIsInCombat(false)
    , bIsAiming(false)
    , bIsAttacking(false)
    , ExhaustionAlpha(0.f)
    , InjuryAlpha(0.f)
    , FearAlpha(0.f)
    , LeftFootIKLocation(FVector::ZeroVector)
    , RightFootIKLocation(FVector::ZeroVector)
    , FootIKAlpha(0.f)
    , SprintThreshold(400.f)
    , SpeedSmoothingRate(10.f)
    , FootIKTraceDistance(80.f)
    , OwnerCharacter(nullptr)
    , MovementComponent(nullptr)
{
}

// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn)
    {
        return;
    }

    OwnerCharacter = Cast<ACharacter>(Pawn);
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        // Re-attempt cache in case character was not ready at init
        NativeInitializeAnimation();
        return;
    }

    // ── Velocity / Speed ─────────────────────────────────────────────────────
    const FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D();

    // Smooth speed to avoid blend-space jitter on direction changes
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, SpeedSmoothingRate);

    // ── Direction ────────────────────────────────────────────────────────────
    Direction = ComputeDirection(Velocity, OwnerCharacter->GetActorRotation());

    // ── Boolean locomotion flags ─────────────────────────────────────────────
    bIsInAir      = MovementComponent->IsFalling();
    bIsCrouching  = OwnerCharacter->bIsCrouched;
    bIsSprinting  = (Speed > SprintThreshold) && !bIsInAir;
    bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.f;

    // ── Foot IK ──────────────────────────────────────────────────────────────
    // Only apply IK when grounded and moving slowly (not sprinting)
    if (!bIsInAir)
    {
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, 1.f, DeltaSeconds, 8.f);
        LeftFootIKLocation  = TraceFootIKLocation(FName("foot_l"));
        RightFootIKLocation = TraceFootIKLocation(FName("foot_r"));
    }
    else
    {
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, 0.f, DeltaSeconds, 8.f);
    }

    // ── Survival stat alphas ─────────────────────────────────────────────────
    // These are driven externally by the character's survival component.
    // Defaults remain 0 unless the character sets them via the public properties.
    // (ExhaustionAlpha, InjuryAlpha, FearAlpha are written by TranspersonalCharacter)
}

// ─────────────────────────────────────────────────────────────────────────────
FVector UPlayerAnimInstance::TraceFootIKLocation(FName BoneName) const
{
    if (!OwnerCharacter)
    {
        return FVector::ZeroVector;
    }

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        return FVector::ZeroVector;
    }

    const FVector BoneLocation = Mesh->GetBoneLocation(BoneName);
    const FVector TraceStart   = BoneLocation + FVector(0.f, 0.f, FootIKTraceDistance * 0.5f);
    const FVector TraceEnd     = BoneLocation - FVector(0.f, 0.f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        Params
    );

    return bHit ? HitResult.Location : BoneLocation;
}

// ─────────────────────────────────────────────────────────────────────────────
float UPlayerAnimInstance::ComputeDirection(const FVector& Velocity, const FRotator& ActorRotation) const
{
    if (Velocity.IsNearlyZero())
    {
        return 0.f;
    }

    const FRotator VelocityRotation = Velocity.Rotation();
    const FRotator DeltaRotation    = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, ActorRotation);
    return DeltaRotation.Yaw;
}
