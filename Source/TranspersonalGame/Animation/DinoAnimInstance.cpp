#include "DinoAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoAnimInstance::UDinoAnimInstance()
{
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsSprinting = false;
    bIsAttacking = false;
    bIsHit = false;
    bIsDead = false;
    bIsEating = false;
    bIsRoaring = false;
    AlertLevel = 0.0f;

    LeftFootIKLocation = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    IKAlpha = 0.0f;
    IKAlphaTarget = 0.0f;
    IKAlphaInterpSpeed = 5.0f;

    OwnerPawn = nullptr;
    MovementComp = nullptr;
}

void UDinoAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        MovementComp = OwnerPawn->FindComponentByClass<UMovementComponent>();
    }
}

void UDinoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn) return;
    }

    if (!MovementComp)
    {
        MovementComp = OwnerPawn->FindComponentByClass<UMovementComponent>();
    }

    // ── Locomotion ──────────────────────────────────────────────────────────
    FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size2D();

    // Direction: dot product of forward vs velocity lateral component
    FVector Forward = OwnerPawn->GetActorForwardVector();
    FVector Right = OwnerPawn->GetActorRightVector();
    FVector VelNorm = Velocity.GetSafeNormal2D();
    Direction = FVector::DotProduct(VelNorm, Right);

    // Air state via CharacterMovementComponent if available
    UCharacterMovementComponent* CharMove = Cast<UCharacterMovementComponent>(MovementComp);
    if (CharMove)
    {
        bIsInAir = CharMove->IsFalling();
        bIsSprinting = (Speed > 400.0f);
    }
    else if (MovementComp)
    {
        bIsInAir = MovementComp->IsFalling();
        bIsSprinting = (Speed > 400.0f);
    }

    // ── IK ──────────────────────────────────────────────────────────────────
    UpdateFootIK(DeltaSeconds);
    UpdateIKAlpha(DeltaSeconds);
}

void UDinoAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    USkeletalMeshComponent* Mesh = GetSkelMeshComponent();
    if (!Mesh) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Trace down from approximate foot bone locations
    // Foot socket names vary by dino mesh — use generic offsets from actor root
    FVector ActorLoc = OwnerPawn->GetActorLocation();
    FVector ActorFwd = OwnerPawn->GetActorForwardVector();
    FVector ActorRight = OwnerPawn->GetActorRightVector();

    float FootSpread = 50.0f;
    float TraceHeight = 200.0f;
    float TraceDepth = 300.0f;

    auto TraceFootIK = [&](FVector FootOffset) -> FVector
    {
        FVector Start = ActorLoc + FootOffset + FVector(0, 0, TraceHeight);
        FVector End = ActorLoc + FootOffset - FVector(0, 0, TraceDepth);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerPawn);

        if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
        {
            return Hit.ImpactPoint;
        }
        return ActorLoc + FootOffset;
    };

    LeftFootIKLocation  = TraceFootIK(ActorRight * -FootSpread);
    RightFootIKLocation = TraceFootIK(ActorRight *  FootSpread);
}

void UDinoAnimInstance::UpdateIKAlpha(float DeltaSeconds)
{
    // Disable IK when sprinting or airborne for performance
    if (bIsInAir || bIsSprinting || Speed > 350.0f)
    {
        IKAlphaTarget = 0.0f;
    }
    else
    {
        IKAlphaTarget = 1.0f;
    }

    IKAlpha = FMath::FInterpTo(IKAlpha, IKAlphaTarget, DeltaSeconds, IKAlphaInterpSpeed);
}

// ── Trigger functions ────────────────────────────────────────────────────────

void UDinoAnimInstance::TriggerAttack()
{
    if (bIsDead) return;
    bIsAttacking = true;
    // Reset after a short delay — Blueprint montage end event clears this
}

void UDinoAnimInstance::TriggerHit()
{
    if (bIsDead) return;
    bIsHit = true;
}

void UDinoAnimInstance::TriggerDeath()
{
    bIsDead = true;
    bIsAttacking = false;
    bIsHit = false;
    bIsRoaring = false;
    bIsEating = false;
}

void UDinoAnimInstance::TriggerRoar()
{
    if (bIsDead || bIsAttacking) return;
    bIsRoaring = true;
}
