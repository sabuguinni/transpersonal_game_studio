#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        OwnerCharacter = Cast<ACharacter>(Pawn);
    }
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        APawn* Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            OwnerCharacter = Cast<ACharacter>(Pawn);
        }
        if (!OwnerCharacter) return;
    }

    UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
    if (!MovComp) return;

    // ── Locomotion ──────────────────────────────────────────────────────────

    FVector Velocity = OwnerCharacter->GetVelocity();
    Velocity.Z = 0.f;
    GroundSpeed = Velocity.Size();

    // Strafe direction: project velocity onto right vector
    FVector RightVec = OwnerCharacter->GetActorRightVector();
    float Dot = FVector::DotProduct(Velocity.GetSafeNormal(), RightVec);
    StrafeDirection = Dot; // -1=left, 0=forward, 1=right

    bIsInAir = MovComp->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;
    bIsSprinting = (GroundSpeed > 350.f) && !bIsInAir;
    bIsClimbing = (MovComp->MovementMode == MOVE_Custom);

    // ── Survival State ───────────────────────────────────────────────────────
    // These will be driven by TranspersonalCharacter survival stats
    // For now, default to full health/stamina unless character exposes them
    bIsLimping = (HealthRatio < 0.3f);

    // ── Aim Offset ──────────────────────────────────────────────────────────
    AController* Controller = OwnerCharacter->GetController();
    if (Controller)
    {
        FRotator ControlRot = Controller->GetControlRotation();
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

        AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.f, 90.f);
        AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.f, 90.f);
    }

    // ── Foot IK ──────────────────────────────────────────────────────────────
    if (!bIsInAir)
    {
        UpdateFootIK();
    }
    else
    {
        // Reset IK when airborne
        LeftFootIKTarget = FVector::ZeroVector;
        RightFootIKTarget = FVector::ZeroVector;
        PelvisOffset = 0.f;
    }
}

void UDinoSurvivorAnimInstance::UpdateFootIK()
{
    if (!OwnerCharacter) return;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return;

    const float TraceLength = 80.f;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    // Left foot socket approximate offset
    FVector LeftFootBase = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorRightVector() * -25.f;
    FVector RightFootBase = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorRightVector() * 25.f;

    FHitResult LeftHit, RightHit;

    // Left foot trace
    bool bLeftHit = World->LineTraceSingleByChannel(
        LeftHit,
        LeftFootBase + FVector(0, 0, TraceLength),
        LeftFootBase - FVector(0, 0, TraceLength),
        ECC_Visibility,
        Params
    );

    // Right foot trace
    bool bRightHit = World->LineTraceSingleByChannel(
        RightHit,
        RightFootBase + FVector(0, 0, TraceLength),
        RightFootBase - FVector(0, 0, TraceLength),
        ECC_Visibility,
        Params
    );

    FVector TargetLeft = bLeftHit ? LeftHit.ImpactPoint : LeftFootBase;
    FVector TargetRight = bRightHit ? RightHit.ImpactPoint : RightFootBase;

    // Smooth interpolation
    float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
    LeftFootIKTarget = FMath::VInterpTo(LeftFootIKTarget, TargetLeft, DeltaTime, IKInterpSpeed);
    RightFootIKTarget = FMath::VInterpTo(RightFootIKTarget, TargetRight, DeltaTime, IKInterpSpeed);

    // Pelvis offset: lower pelvis to accommodate the lower foot
    float LeftDelta = TargetLeft.Z - OwnerCharacter->GetActorLocation().Z;
    float RightDelta = TargetRight.Z - OwnerCharacter->GetActorLocation().Z;
    float TargetPelvis = FMath::Min(LeftDelta, RightDelta);
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvis, DeltaTime, IKInterpSpeed);
}
