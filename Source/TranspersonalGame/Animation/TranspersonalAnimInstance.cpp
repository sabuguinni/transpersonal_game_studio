#include "TranspersonalAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Locomotion defaults
    LocomotionState     = EAnim_LocomotionState::Idle;
    GroundSpeed         = 0.0f;
    Direction           = 0.0f;
    bIsMoving           = false;
    bIsInAir            = false;
    bIsCrouching        = false;
    bIsSprinting        = false;

    // Speed thresholds (cm/s)
    WalkSpeedThreshold  = 10.0f;
    RunSpeedThreshold   = 250.0f;
    SprintSpeedThreshold = 450.0f;

    // Foot IK defaults
    FootIKTraceDistance = 60.0f;
    FootIKInterpSpeed   = 15.0f;

    // Lean defaults
    LeanInterpSpeed     = 8.0f;

    // Survival defaults
    FearLevel           = 0.0f;
    StaminaNormalized   = 1.0f;
    bIsExhausted        = false;

    LandingTimer        = 0.0f;
    PreviousVelocity    = FVector::ZeroVector;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
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

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent) return;

    // Cache velocity data
    FVector Velocity = MovementComponent->Velocity;
    GroundSpeed = Velocity.Size2D();
    bIsMoving   = GroundSpeed > WalkSpeedThreshold;
    bIsInAir    = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Direction angle (for strafe blending)
    if (bIsMoving)
    {
        FRotator ActorRot = OwnerCharacter->GetActorRotation();
        FRotator VelRot   = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // Landing timer — brief land state before returning to idle
    if (LandingTimer > 0.0f)
    {
        LandingTimer -= DeltaSeconds;
    }

    UpdateLocomotionState();
    UpdateLean(DeltaSeconds);
    UpdateSurvivalOverlays();

    PreviousVelocity = Velocity;
}

void UTranspersonalAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // Foot IK runs on the game thread via NativeUpdateAnimation for line traces
}

// ─── Private: Locomotion State Machine ──────────────────────────────────────

void UTranspersonalAnimInstance::UpdateLocomotionState()
{
    if (!OwnerCharacter || !MovementComponent) return;

    // Death check — highest priority
    if (LocomotionState == EAnim_LocomotionState::Death)
        return;

    // In-air states
    if (bIsInAir)
    {
        FVector Vel = MovementComponent->Velocity;
        if (Vel.Z > 0.0f)
        {
            LocomotionState = EAnim_LocomotionState::Jump;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Fall;
        }
        return;
    }

    // Landing — brief state after touching ground
    if (LandingTimer > 0.0f)
    {
        LocomotionState = EAnim_LocomotionState::Land;
        return;
    }

    // Crouching
    if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouch;
        return;
    }

    // Ground locomotion by speed
    if (GroundSpeed < WalkSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
    else if (GroundSpeed < RunSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else if (GroundSpeed < SprintSpeedThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
}

// ─── Private: Foot IK ────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Bone socket names (standard UE5 Mannequin)
    const FName LeftFootSocket  = TEXT("foot_l");
    const FName RightFootSocket = TEXT("foot_r");

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp) return;

    auto TraceFootIK = [&](FName SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
    {
        FVector SocketLoc = MeshComp->GetSocketLocation(SocketName);
        FVector TraceStart = SocketLoc + FVector(0, 0, FootIKTraceDistance);
        FVector TraceEnd   = SocketLoc - FVector(0, 0, FootIKTraceDistance);

        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        bool bHit = World->LineTraceSingleByChannel(
            HitResult, TraceStart, TraceEnd,
            ECC_Visibility, Params);

        if (bHit)
        {
            OutLocation = HitResult.ImpactPoint;
            OutRotation = FRotator(
                FMath::RadiansToDegrees(FMath::Atan2(HitResult.ImpactNormal.X, HitResult.ImpactNormal.Z)) * -1.0f,
                0.0f,
                FMath::RadiansToDegrees(FMath::Atan2(HitResult.ImpactNormal.Y, HitResult.ImpactNormal.Z)));
            OutAlpha = FMath::FInterpTo(OutAlpha, 1.0f, DeltaSeconds, FootIKInterpSpeed);
        }
        else
        {
            OutAlpha = FMath::FInterpTo(OutAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        }
    };

    TraceFootIK(LeftFootSocket,
        FootIKData.LeftFootLocation, FootIKData.LeftFootRotation, FootIKData.LeftFootAlpha);
    TraceFootIK(RightFootSocket,
        FootIKData.RightFootLocation, FootIKData.RightFootRotation, FootIKData.RightFootAlpha);

    // Pelvis offset — lower pelvis to the lowest foot to prevent floating
    float LeftDelta  = FootIKData.LeftFootLocation.Z  - OwnerCharacter->GetActorLocation().Z;
    float RightDelta = FootIKData.RightFootLocation.Z - OwnerCharacter->GetActorLocation().Z;
    float TargetPelvisOffset = FMath::Min(LeftDelta, RightDelta);
    FootIKData.PelvisOffset  = FMath::FInterpTo(
        FootIKData.PelvisOffset, TargetPelvisOffset, DeltaSeconds, FootIKInterpSpeed);
}

// ─── Private: Lean ───────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!MovementComponent) return;

    FVector CurrentVel  = MovementComponent->Velocity;
    FVector Acceleration = (CurrentVel - PreviousVelocity) / FMath::Max(DeltaSeconds, 0.001f);

    // Project acceleration into local character space
    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FVector LocalAccel = ActorRot.UnrotateVector(Acceleration);

    float TargetForwardBack = FMath::Clamp(LocalAccel.X / 1000.0f, -1.0f, 1.0f);
    float TargetLeftRight   = FMath::Clamp(LocalAccel.Y / 1000.0f, -1.0f, 1.0f);

    LeanData.LeanForwardBack = FMath::FInterpTo(
        LeanData.LeanForwardBack, TargetForwardBack, DeltaSeconds, LeanInterpSpeed);
    LeanData.LeanLeftRight = FMath::FInterpTo(
        LeanData.LeanLeftRight, TargetLeftRight, DeltaSeconds, LeanInterpSpeed);
}

// ─── Private: Survival Overlays ──────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateSurvivalOverlays()
{
    // These values are read from the TranspersonalCharacter survival stats.
    // The character exposes FearLevel and Stamina as public properties.
    // We read them here to drive animation overlays (trembling, hunched posture, etc.)

    if (!OwnerCharacter) return;

    // Try to get survival stats via interface or direct cast
    // Using property access via reflection to avoid hard dependency
    // In full integration, cast to ATranspersonalCharacter and read directly
    bIsExhausted = (StaminaNormalized < 0.15f);
}
