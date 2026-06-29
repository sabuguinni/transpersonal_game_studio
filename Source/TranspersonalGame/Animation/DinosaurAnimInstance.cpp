#include "DinosaurAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    // Locomotion defaults
    Speed = 0.f;
    SmoothedSpeed = 0.f;
    ForwardSpeed = 0.f;
    LateralSpeed = 0.f;
    bIsInAir = false;
    bIsAccelerating = false;
    bIsSprinting = false;
    LocomotionAlpha = 0.f;

    // Combat defaults
    bIsAttacking = false;
    bIsAlert = false;
    bIsChasing = false;
    bIsRoaring = false;
    bIsFeeding = false;
    bIsDead = false;
    HealthAlpha = 1.f;
    InjuryAlpha = 0.f;

    // IK defaults
    bEnableFootIK = true;
    LeftFrontFootIKTarget = FVector::ZeroVector;
    RightFrontFootIKTarget = FVector::ZeroVector;
    LeftRearFootIKTarget = FVector::ZeroVector;
    RightRearFootIKTarget = FVector::ZeroVector;
    BodyLeanAlpha = 0.f;

    // Head tracking defaults
    bIsHeadTracking = false;
    HeadLookAtTarget = FVector::ZeroVector;
    HeadTrackingWeight = 0.f;

    // Config defaults — suitable for a medium-sized biped (Raptor-class)
    SprintSpeedThreshold = 600.f;
    WalkSpeedThreshold = 150.f;
    SpeedSmoothingRate = 8.f;
    FootCount = 2;

    // Internal state
    bWasAlert = false;
    bWasAttacking = false;
    bWasDead = false;
    bWasRoaring = false;
    OwnerPawn = nullptr;
    MovementComponent = nullptr;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        return;
    }

    // Try to get movement component — works for ACharacter subclasses
    ACharacter* OwnerChar = Cast<ACharacter>(OwnerPawn);
    if (OwnerChar)
    {
        MovementComponent = OwnerChar->GetCharacterMovement();
    }

    // Initialise foot IK targets to actor location
    const FVector ActorLoc = OwnerPawn->GetActorLocation();
    LeftFrontFootIKTarget  = ActorLoc;
    RightFrontFootIKTarget = ActorLoc;
    LeftRearFootIKTarget   = ActorLoc;
    RightRearFootIKTarget  = ActorLoc;
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn || DeltaSeconds <= 0.f)
    {
        return;
    }

    // ─── Locomotion ───────────────────────────────────────────────────────────

    const FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size2D();

    // Smooth speed to avoid blend space jitter
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, SpeedSmoothingRate);

    // Compute locomotion alpha (0=idle, 0.5=walk, 1=sprint)
    LocomotionAlpha = ComputeLocomotionAlpha(SmoothedSpeed);

    // Forward / lateral components in local space
    const FVector LocalVelocity = OwnerPawn->GetActorTransform().InverseTransformVector(Velocity);
    const float MaxSpeed = FMath::Max(SprintSpeedThreshold, 1.f);
    ForwardSpeed = FMath::Clamp(LocalVelocity.X / MaxSpeed, -1.f, 1.f);
    LateralSpeed = FMath::Clamp(LocalVelocity.Y / MaxSpeed, -1.f, 1.f);

    bIsAccelerating = Speed > 10.f;
    bIsSprinting    = Speed >= SprintSpeedThreshold;

    if (MovementComponent)
    {
        bIsInAir = MovementComponent->IsFalling();
    }

    // ─── Injury blend ─────────────────────────────────────────────────────────
    // InjuryAlpha is set externally by the NPC/Combat system via property.
    // Here we just clamp it to be safe.
    InjuryAlpha = FMath::Clamp(InjuryAlpha, 0.f, 1.f);
    HealthAlpha = FMath::Clamp(HealthAlpha, 0.f, 1.f);

    // ─── State transition events ──────────────────────────────────────────────

    if (bIsAlert && !bWasAlert)
    {
        OnAlertStateEntered();
    }
    bWasAlert = bIsAlert;

    if (bIsAttacking && !bWasAttacking)
    {
        OnAttackStarted();
    }
    bWasAttacking = bIsAttacking;

    if (bIsDead && !bWasDead)
    {
        OnDeathStarted();
    }
    bWasDead = bIsDead;

    if (bIsRoaring && !bWasRoaring)
    {
        OnRoarStarted();
    }
    bWasRoaring = bIsRoaring;

    // ─── IK & Head Tracking ───────────────────────────────────────────────────

    if (bEnableFootIK && !bIsInAir && !bIsDead)
    {
        UpdateFootIK();
    }

    if (bIsHeadTracking)
    {
        UpdateHeadTracking();
    }
    else
    {
        // Fade out head tracking weight when not tracking
        HeadTrackingWeight = FMath::FInterpTo(HeadTrackingWeight, 0.f, DeltaSeconds, 4.f);
    }
}

float UDinosaurAnimInstance::ComputeLocomotionAlpha(float CurrentSpeed) const
{
    if (CurrentSpeed < WalkSpeedThreshold)
    {
        // Idle to walk range: 0 → 0.5
        return FMath::GetMappedRangeValueClamped(
            FVector2D(0.f, WalkSpeedThreshold),
            FVector2D(0.f, 0.5f),
            CurrentSpeed
        );
    }
    else
    {
        // Walk to sprint range: 0.5 → 1.0
        return FMath::GetMappedRangeValueClamped(
            FVector2D(WalkSpeedThreshold, SprintSpeedThreshold),
            FVector2D(0.5f, 1.f),
            CurrentSpeed
        );
    }
}

void UDinosaurAnimInstance::UpdateFootIK()
{
    if (!OwnerPawn)
    {
        return;
    }

    UWorld* World = OwnerPawn->GetWorld();
    if (!World)
    {
        return;
    }

    // Trace parameters — ignore the owning pawn
    FCollisionQueryParams TraceParams(FName(TEXT("DinoFootIK")), true, OwnerPawn);
    TraceParams.bReturnPhysicalMaterial = false;

    const FVector ActorLoc    = OwnerPawn->GetActorLocation();
    const FTransform ActorXf  = OwnerPawn->GetActorTransform();
    const float TraceHalfHeight = 100.f;
    const float TraceRadius     = 10.f;

    // Foot socket offsets in local space (approximate for biped/quadruped)
    // These will be overridden per-species in Blueprint child classes
    TArray<FVector> FootOffsets;
    if (FootCount >= 4)
    {
        // Quadruped: front-left, front-right, rear-left, rear-right
        FootOffsets.Add(FVector( 80.f, -40.f, 0.f));
        FootOffsets.Add(FVector( 80.f,  40.f, 0.f));
        FootOffsets.Add(FVector(-80.f, -40.f, 0.f));
        FootOffsets.Add(FVector(-80.f,  40.f, 0.f));
    }
    else
    {
        // Biped: left, right (front feet only used)
        FootOffsets.Add(FVector(20.f, -25.f, 0.f));
        FootOffsets.Add(FVector(20.f,  25.f, 0.f));
    }

    TArray<FVector*> IKTargets = {
        &LeftFrontFootIKTarget,
        &RightFrontFootIKTarget,
        &LeftRearFootIKTarget,
        &RightRearFootIKTarget
    };

    for (int32 i = 0; i < FootOffsets.Num() && i < IKTargets.Num(); ++i)
    {
        const FVector WorldFootPos = ActorXf.TransformPosition(FootOffsets[i]);
        const FVector TraceStart   = WorldFootPos + FVector(0.f, 0.f,  TraceHalfHeight);
        const FVector TraceEnd     = WorldFootPos + FVector(0.f, 0.f, -TraceHalfHeight);

        FHitResult HitResult;
        const bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_WorldStatic,
            TraceParams
        );

        if (bHit)
        {
            // Smooth the IK target towards the hit point
            *IKTargets[i] = FMath::VInterpTo(*IKTargets[i], HitResult.ImpactPoint, GetWorld()->GetDeltaSeconds(), 12.f);
        }
        else
        {
            // No ground found — reset to actor base
            *IKTargets[i] = FMath::VInterpTo(*IKTargets[i], ActorLoc, GetWorld()->GetDeltaSeconds(), 6.f);
        }
    }

    // Compute body lean from slope angle
    const FVector SlopeNormal = (LeftFrontFootIKTarget - LeftRearFootIKTarget).GetSafeNormal();
    const float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SlopeNormal, FVector::UpVector)));
    BodyLeanAlpha = FMath::Clamp(SlopeAngle / 45.f, 0.f, 1.f);
}

void UDinosaurAnimInstance::UpdateHeadTracking()
{
    if (!OwnerPawn)
    {
        return;
    }

    // Head tracking weight fades in when bIsHeadTracking is true
    HeadTrackingWeight = FMath::FInterpTo(HeadTrackingWeight, 1.f, GetWorld()->GetDeltaSeconds(), 4.f);

    // HeadLookAtTarget is set externally by the AI/BehaviorTree system.
    // Here we just ensure the weight is correct.
    // The actual IK solving is done in the AnimBlueprint using LookAt node.
}
