#include "DinosaurAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UDinosaurAnimInstance::UDinosaurAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    SmoothedSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsInAir = false;
    bIsMoving = false;
    bIsSprinting = false;
    NormalisedSpeed = 0.0f;

    // State defaults
    LocomotionState = EAnim_DinoLocomotionState::Idle;
    PreviousLocomotionState = EAnim_DinoLocomotionState::Idle;
    SizeCategory = EAnim_DinoSizeCategory::Medium;

    // Combat defaults
    bIsAttacking = false;
    bIsAlert = false;
    HealthAlpha = 1.0f;
    bIsWounded = false;

    // IK defaults
    IK_FrontLeftFoot = FVector::ZeroVector;
    IK_FrontRightFoot = FVector::ZeroVector;
    IK_RearLeftFoot = FVector::ZeroVector;
    IK_RearRightFoot = FVector::ZeroVector;
    SlopeLeanAlpha = 0.0f;
    PelvisOffset = 0.0f;

    // Behaviour defaults
    bIsEating = false;
    bIsSleeping = false;
    bIsRoaring = false;
    TailSwayAlpha = 0.0f;
    TailSwayTime = 0.0f;

    // Playback
    PlayRate = 1.0f;

    // Config defaults — tuned for medium dino (Dilophosaurus-scale)
    MaxRunSpeed = 900.0f;
    WalkSpeedThreshold = 50.0f;
    SprintSpeedThreshold = 600.0f;
    IKTraceDistance = 80.0f;
    SpeedSmoothingAlpha = 0.12f;

    OwnerPawn = nullptr;
    MovementComp = nullptr;
}

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerPawn = TryGetPawnOwner();
    if (OwnerPawn)
    {
        MovementComp = OwnerPawn->FindComponentByClass<UMovementComponent>();

        // Adjust defaults based on size category
        switch (SizeCategory)
        {
        case EAnim_DinoSizeCategory::Small:
            MaxRunSpeed = 1200.0f;
            SprintSpeedThreshold = 800.0f;
            IKTraceDistance = 50.0f;
            break;
        case EAnim_DinoSizeCategory::Large:
            MaxRunSpeed = 700.0f;
            SprintSpeedThreshold = 500.0f;
            IKTraceDistance = 120.0f;
            break;
        case EAnim_DinoSizeCategory::Massive:
            MaxRunSpeed = 400.0f;
            SprintSpeedThreshold = 280.0f;
            IKTraceDistance = 200.0f;
            break;
        default:
            break;
        }
    }
}

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerPawn)
    {
        OwnerPawn = TryGetPawnOwner();
        if (!OwnerPawn) return;
        MovementComp = OwnerPawn->FindComponentByClass<UMovementComponent>();
    }

    // ── Raw speed from velocity ──────────────────────────────────────────────
    FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size2D();

    // Smooth speed to avoid jitter on uneven terrain
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, 1.0f / FMath::Max(SpeedSmoothingAlpha, 0.001f));

    // Normalised 0..1
    NormalisedSpeed = (MaxRunSpeed > 0.0f) ? FMath::Clamp(SmoothedSpeed / MaxRunSpeed, 0.0f, 1.0f) : 0.0f;

    // Movement direction relative to actor forward
    if (Speed > WalkSpeedThreshold)
    {
        FRotator ActorRot = OwnerPawn->GetActorRotation();
        FVector LocalVel = ActorRot.UnrotateVector(Velocity);
        MovementDirection = FMath::RadiansToDegrees(FMath::Atan2(LocalVel.Y, LocalVel.X));
    }
    else
    {
        MovementDirection = 0.0f;
    }

    // ── Air state ────────────────────────────────────────────────────────────
    if (MovementComp)
    {
        bIsInAir = MovementComp->IsFalling();
    }

    // ── Boolean flags ────────────────────────────────────────────────────────
    bIsMoving = Speed > WalkSpeedThreshold;
    bIsSprinting = Speed > SprintSpeedThreshold;
    bIsWounded = HealthAlpha < 0.3f;

    // ── State machine ────────────────────────────────────────────────────────
    PreviousLocomotionState = LocomotionState;
    UpdateLocomotionState();

    // ── Procedural systems ───────────────────────────────────────────────────
    UpdateFootIK(DeltaSeconds);
    UpdatePlayRate();
    UpdateTailSway(DeltaSeconds);
}

void UDinosaurAnimInstance::UpdateLocomotionState()
{
    // Priority: death > sleep > attack > eat > roar > locomotion
    if (HealthAlpha <= 0.0f)
    {
        LocomotionState = EAnim_DinoLocomotionState::Death;
        return;
    }
    if (bIsSleeping)
    {
        LocomotionState = EAnim_DinoLocomotionState::Sleep;
        return;
    }
    if (bIsAttacking)
    {
        LocomotionState = EAnim_DinoLocomotionState::Attack;
        return;
    }
    if (bIsEating)
    {
        LocomotionState = EAnim_DinoLocomotionState::Eat;
        return;
    }
    if (bIsRoaring)
    {
        LocomotionState = EAnim_DinoLocomotionState::Roar;
        return;
    }
    if (bIsAlert && !bIsMoving)
    {
        LocomotionState = EAnim_DinoLocomotionState::Investigate;
        return;
    }

    // Locomotion tiers
    if (!bIsMoving)
    {
        LocomotionState = EAnim_DinoLocomotionState::Idle;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_DinoLocomotionState::Run;
    }
    else if (Speed > WalkSpeedThreshold * 3.0f)
    {
        LocomotionState = EAnim_DinoLocomotionState::Trot;
    }
    else
    {
        LocomotionState = EAnim_DinoLocomotionState::Walk;
    }
}

void UDinosaurAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerPawn) return;

    UWorld* World = OwnerPawn->GetWorld();
    if (!World) return;

    // Only perform IK traces when on the ground and not sleeping
    if (bIsInAir || bIsSleeping) return;

    FCollisionQueryParams TraceParams(FName(TEXT("DinoFootIK")), true, OwnerPawn);
    TraceParams.bReturnPhysicalMaterial = false;

    // Helper lambda — traces downward from a socket-offset position
    auto TraceFootIK = [&](FVector FootWorldPos) -> FVector
    {
        FVector TraceStart = FootWorldPos + FVector(0.0f, 0.0f, IKTraceDistance * 0.5f);
        FVector TraceEnd   = FootWorldPos - FVector(0.0f, 0.0f, IKTraceDistance);
        FHitResult Hit;
        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, TraceParams))
        {
            return Hit.ImpactPoint;
        }
        return FootWorldPos;
    };

    FVector ActorLoc = OwnerPawn->GetActorLocation();
    FVector ActorFwd = OwnerPawn->GetActorForwardVector();
    FVector ActorRight = OwnerPawn->GetActorRightVector();

    // Approximate foot positions based on actor bounds — refined per species in BP
    float HalfLength = 80.0f;
    float HalfWidth  = 40.0f;

    IK_FrontLeftFoot  = TraceFootIK(ActorLoc + ActorFwd * HalfLength - ActorRight * HalfWidth);
    IK_FrontRightFoot = TraceFootIK(ActorLoc + ActorFwd * HalfLength + ActorRight * HalfWidth);
    IK_RearLeftFoot   = TraceFootIK(ActorLoc - ActorFwd * HalfLength - ActorRight * HalfWidth);
    IK_RearRightFoot  = TraceFootIK(ActorLoc - ActorFwd * HalfLength + ActorRight * HalfWidth);

    // Compute pelvis offset from average foot height delta
    float AvgFootZ = (IK_FrontLeftFoot.Z + IK_FrontRightFoot.Z + IK_RearLeftFoot.Z + IK_RearRightFoot.Z) * 0.25f;
    float TargetPelvisOffset = AvgFootZ - ActorLoc.Z;
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaSeconds, 10.0f);

    // Slope lean: compare front vs rear foot heights
    float FrontAvgZ = (IK_FrontLeftFoot.Z + IK_FrontRightFoot.Z) * 0.5f;
    float RearAvgZ  = (IK_RearLeftFoot.Z  + IK_RearRightFoot.Z)  * 0.5f;
    float SlopeDelta = FMath::Abs(FrontAvgZ - RearAvgZ);
    SlopeLeanAlpha = FMath::Clamp(SlopeDelta / 100.0f, 0.0f, 1.0f);
}

void UDinosaurAnimInstance::UpdatePlayRate()
{
    // Larger dinosaurs have slower, weightier animations
    switch (SizeCategory)
    {
    case EAnim_DinoSizeCategory::Small:
        PlayRate = FMath::Lerp(0.9f, 1.4f, NormalisedSpeed);
        break;
    case EAnim_DinoSizeCategory::Medium:
        PlayRate = FMath::Lerp(0.8f, 1.2f, NormalisedSpeed);
        break;
    case EAnim_DinoSizeCategory::Large:
        PlayRate = FMath::Lerp(0.6f, 1.0f, NormalisedSpeed);
        break;
    case EAnim_DinoSizeCategory::Massive:
        PlayRate = FMath::Lerp(0.4f, 0.8f, NormalisedSpeed);
        break;
    default:
        PlayRate = 1.0f;
        break;
    }

    // Wounded dinos move slower
    if (bIsWounded)
    {
        PlayRate *= 0.7f;
    }
}

void UDinosaurAnimInstance::UpdateTailSway()
{
    // Tail sway is driven by a sine oscillation — faster when running
    float SwayFrequency = FMath::Lerp(0.5f, 2.5f, NormalisedSpeed);
    TailSwayTime += 0.016f * SwayFrequency; // approximate DeltaSeconds
    TailSwayAlpha = FMath::Sin(TailSwayTime);
}

void UDinosaurAnimInstance::UpdateTailSway(float DeltaSeconds)
{
    float SwayFrequency = FMath::Lerp(0.5f, 2.5f, NormalisedSpeed);
    TailSwayTime += DeltaSeconds * SwayFrequency;
    TailSwayAlpha = FMath::Sin(TailSwayTime);
}
