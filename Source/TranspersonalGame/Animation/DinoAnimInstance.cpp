#include "DinoAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoAnimInstance::UDinoAnimInstance()
    : Speed(0.f)
    , Direction(0.f)
    , bIsInAir(false)
    , bIsAttacking(false)
    , bIsEating(false)
    , bIsAlert(false)
    , LeftFootIKLocation(FVector::ZeroVector)
    , RightFootIKLocation(FVector::ZeroVector)
    , PelvisOffset(0.f)
    , TailSwayAlpha(0.f)
    , OwnerPawn(nullptr)
    , MovementComp(nullptr)
    , PelvisOffsetTarget(0.f)
{
}

void UDinoAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        ACharacter* Char = Cast<ACharacter>(OwnerPawn);
        if (Char)
        {
            MovementComp = Char->GetCharacterMovement();
        }
    }
}

void UDinoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        return;
    }

    // ── Locomotion ──────────────────────────────────────────────────────────
    const FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size2D();

    // Direction: signed angle between actor forward and velocity
    if (Speed > 1.f)
    {
        const FVector Forward = OwnerPawn->GetActorForwardVector();
        const FVector VelNorm = Velocity.GetSafeNormal2D();
        const float Dot   = FVector::DotProduct(Forward, VelNorm);
        const FVector Cross = FVector::CrossProduct(Forward, VelNorm);
        Direction = FMath::RadiansToDegrees(FMath::Atan2(Cross.Z, Dot));
    }
    else
    {
        Direction = 0.f;
    }

    // ── Air state ───────────────────────────────────────────────────────────
    if (MovementComp)
    {
        bIsInAir = MovementComp->IsFalling();
    }

    // ── Tail sway — driven by normalised speed ───────────────────────────────
    // Assume max dino speed ~1200 cm/s (raptor sprint)
    TailSwayAlpha = FMath::Clamp(Speed / 1200.f, 0.f, 1.f);

    // ── IK Foot Placement ───────────────────────────────────────────────────
    if (!bIsInAir)
    {
        LeftFootIKLocation  = TraceFootIK(TEXT("foot_l"));
        RightFootIKLocation = TraceFootIK(TEXT("foot_r"));

        // Pelvis offset: average of foot deltas from capsule base
        const float OwnerZ = OwnerPawn->GetActorLocation().Z;
        const float LeftDelta  = LeftFootIKLocation.Z  - OwnerZ;
        const float RightDelta = RightFootIKLocation.Z - OwnerZ;
        PelvisOffsetTarget = FMath::Min(LeftDelta, RightDelta);
    }
    else
    {
        PelvisOffsetTarget = 0.f;
    }

    // Smooth pelvis offset interpolation
    PelvisOffset = FMath::FInterpTo(PelvisOffset, PelvisOffsetTarget, DeltaSeconds, 10.f);
}

FVector UDinoAnimInstance::TraceFootIK(FName SocketName) const
{
    if (!OwnerPawn)
    {
        return FVector::ZeroVector;
    }

    USkeletalMeshComponent* Mesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh)
    {
        return OwnerPawn->GetActorLocation();
    }

    const FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
    const FVector TraceStart = SocketLoc + FVector(0.f, 0.f, 50.f);
    const FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, 75.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    UWorld* World = OwnerPawn->GetWorld();
    if (World && World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        return HitResult.ImpactPoint;
    }

    return SocketLoc;
}

void UDinoAnimInstance::TriggerAttack()
{
    bIsAttacking = true;
    // Montage playback is handled in the Animation Blueprint graph.
    // This flag drives the state machine transition to the Attack state.
}

void UDinoAnimInstance::StartEating()
{
    bIsEating = true;
}

void UDinoAnimInstance::StopEating()
{
    bIsEating = false;
}
