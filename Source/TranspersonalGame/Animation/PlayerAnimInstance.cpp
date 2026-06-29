#include "Animation/PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
    : Speed(0.f)
    , SmoothedSpeed(0.f)
    , Direction(0.f)
    , bIsInAir(false)
    , bIsCrouching(false)
    , bIsSprinting(false)
    , bIsMoving(false)
    , bIsSneaking(false)
    , bIsClimbing(false)
    , FearLevel(0.f)
    , StaminaLevel(1.f)
    , bIsInCombat(false)
    , bIsAttacking(false)
    , AimPitch(0.f)
    , AimYaw(0.f)
    , LeftFootIKLocation(FVector::ZeroVector)
    , RightFootIKLocation(FVector::ZeroVector)
    , PelvisOffset(0.f)
    , bFootIKEnabled(false)
    , MovingThreshold(10.f)
    , SprintThreshold(400.f)
    , SpeedSmoothingRate(10.f)
    , FootIKTraceDistance(55.f)
    , OwnerCharacter(nullptr)
    , MovementComponent(nullptr)
{
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn) return;

    OwnerCharacter   = Cast<ACharacter>(Pawn);
    MovementComponent = OwnerCharacter
                        ? OwnerCharacter->GetCharacterMovement()
                        : nullptr;
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent) return;

    // ── Velocity & Speed ────────────────────────────────────────────────────
    const FVector Velocity    = MovementComponent->Velocity;
    const FVector VelocityXY  = FVector(Velocity.X, Velocity.Y, 0.f);
    Speed = VelocityXY.Size();

    // Smooth speed to avoid blend-space jitter
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, SpeedSmoothingRate);

    // ── Direction ───────────────────────────────────────────────────────────
    if (Speed > MovingThreshold)
    {
        const FRotator ActorRot = OwnerCharacter->GetActorRotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(
            VelocityXY.Rotation(), ActorRot).Yaw;
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.f, DeltaSeconds, 5.f);
    }

    // ── Boolean States ──────────────────────────────────────────────────────
    bIsInAir    = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsMoving   = Speed > MovingThreshold;
    bIsSprinting = Speed > SprintThreshold;
    bIsSneaking  = bIsCrouching && Speed > MovingThreshold && Speed < 150.f;

    // ── Foot IK ─────────────────────────────────────────────────────────────
    bFootIKEnabled = !bIsInAir;
    if (bFootIKEnabled)
    {
        UpdateFootIK();
    }
    else
    {
        // Reset IK when airborne
        LeftFootIKLocation  = FVector::ZeroVector;
        RightFootIKLocation = FVector::ZeroVector;
        PelvisOffset        = FMath::FInterpTo(PelvisOffset, 0.f, DeltaSeconds, 15.f);
    }
}

FVector UPlayerAnimInstance::TraceFootIK(FName SocketName, float& OutPelvisDelta) const
{
    if (!OwnerCharacter) { OutPelvisDelta = 0.f; return FVector::ZeroVector; }

    const USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) { OutPelvisDelta = 0.f; return FVector::ZeroVector; }

    const FVector SocketLoc = Mesh->GetSocketLocation(SocketName);

    // Trace downward from above the foot
    const FVector TraceStart = SocketLoc + FVector(0.f, 0.f, FootIKTraceDistance);
    const FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (bHit)
    {
        OutPelvisDelta = HitResult.ImpactPoint.Z - SocketLoc.Z;
        return HitResult.ImpactPoint;
    }

    OutPelvisDelta = 0.f;
    return SocketLoc;
}

void UPlayerAnimInstance::UpdateFootIK()
{
    float LeftDelta  = 0.f;
    float RightDelta = 0.f;

    LeftFootIKLocation  = TraceFootIK(FName("foot_l"), LeftDelta);
    RightFootIKLocation = TraceFootIK(FName("foot_r"), RightDelta);

    // Pelvis drops to accommodate the lower foot
    const float TargetPelvisOffset = FMath::Min(LeftDelta, RightDelta);
    const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaSeconds, 15.f);
}
