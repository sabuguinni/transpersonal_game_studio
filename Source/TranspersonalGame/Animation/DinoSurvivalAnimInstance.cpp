#include "DinoSurvivalAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivalAnimInstance::UDinoSurvivalAnimInstance()
{
    // Movement defaults
    GroundSpeed        = 0.f;
    bIsMoving          = false;
    bIsInAir           = false;
    bIsSprinting       = false;
    bIsCrouching       = false;
    MovementDirection  = 0.f;
    VerticalVelocity   = 0.f;

    // Survival defaults
    Health             = 100.f;
    Stamina            = 100.f;
    bIsInjured         = false;
    bIsExhausted       = false;

    // IK defaults
    LeftFootIKLocation  = FVector::ZeroVector;
    RightFootIKLocation = FVector::ZeroVector;
    PelvisOffset        = 0.f;

    // Config defaults
    RunThreshold  = 375.f;   // cm/s — roughly 3.75 m/s
    IdleThreshold = 10.f;

    OwnerCharacter    = nullptr;
    MovementComponent = nullptr;
}

void UDinoSurvivalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UDinoSurvivalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        // Re-attempt cache in case character was late-spawned
        OwnerCharacter    = Cast<ACharacter>(GetOwningActor());
        MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
        return;
    }

    // ── Velocity ─────────────────────────────────────────────────────────────
    const FVector Velocity = MovementComponent->Velocity;
    GroundSpeed            = Velocity.Size2D();   // horizontal speed only
    VerticalVelocity       = Velocity.Z;

    // ── Boolean states ────────────────────────────────────────────────────────
    bIsMoving    = GroundSpeed > IdleThreshold;
    bIsInAir     = MovementComponent->IsFalling();
    bIsSprinting = GroundSpeed > RunThreshold;
    bIsCrouching = MovementComponent->IsCrouching();

    // ── Movement direction (strafe angle) ─────────────────────────────────────
    if (bIsMoving)
    {
        const FRotator ActorRot  = OwnerCharacter->GetActorRotation();
        const FRotator VelRot    = UKismetMathLibrary::MakeRotFromX(Velocity);
        const FRotator DeltaRot  = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot);
        MovementDirection        = DeltaRot.Yaw;
    }
    else
    {
        MovementDirection = 0.f;
    }

    // ── Survival stats (read from character if it exposes them) ───────────────
    // We use a soft interface — if the character doesn't expose these, defaults hold
    if (OwnerCharacter->GetClass()->ImplementsInterface(UInterface::StaticClass()))
    {
        // Placeholder: survival stats will be wired when TranspersonalCharacter
        // exposes Health/Stamina as BlueprintReadOnly properties.
        // For now, keep at defaults (100/100) so animations play normally.
    }

    bIsInjured   = Health < 30.f;
    bIsExhausted = Stamina < 20.f;

    // ── Foot IK ───────────────────────────────────────────────────────────────
    if (!bIsInAir)
    {
        UpdateFootIK();
        UpdatePelvisOffset();
    }
    else
    {
        // Reset IK when airborne
        LeftFootIKLocation  = FVector::ZeroVector;
        RightFootIKLocation = FVector::ZeroVector;
        PelvisOffset        = 0.f;
    }
}

void UDinoSurvivalAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter) return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    const FVector ActorLoc = OwnerCharacter->GetActorLocation();
    const float   TraceUp  = 50.f;
    const float   TraceDown = 100.f;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    // Left foot — offset relative to character root (approximate socket positions)
    const FVector LeftStart  = ActorLoc + FVector(-20.f, -15.f, TraceUp);
    const FVector LeftEnd    = ActorLoc + FVector(-20.f, -15.f, -TraceDown);

    FHitResult LeftHit;
    if (World->LineTraceSingleByChannel(LeftHit, LeftStart, LeftEnd, ECC_WorldStatic, Params))
    {
        LeftFootIKLocation = LeftHit.ImpactPoint;
    }

    // Right foot
    const FVector RightStart = ActorLoc + FVector(-20.f,  15.f, TraceUp);
    const FVector RightEnd   = ActorLoc + FVector(-20.f,  15.f, -TraceDown);

    FHitResult RightHit;
    if (World->LineTraceSingleByChannel(RightHit, RightStart, RightEnd, ECC_WorldStatic, Params))
    {
        RightFootIKLocation = RightHit.ImpactPoint;
    }
}

void UDinoSurvivalAnimInstance::UpdatePelvisOffset()
{
    if (LeftFootIKLocation.IsZero() || RightFootIKLocation.IsZero())
    {
        PelvisOffset = 0.f;
        return;
    }

    // Pelvis drops to the lower of the two feet so both can reach the ground
    const float LeftDelta  = LeftFootIKLocation.Z  - OwnerCharacter->GetActorLocation().Z;
    const float RightDelta = RightFootIKLocation.Z - OwnerCharacter->GetActorLocation().Z;
    PelvisOffset = FMath::Min(LeftDelta, RightDelta);
    PelvisOffset = FMath::Clamp(PelvisOffset, -30.f, 0.f);
}
