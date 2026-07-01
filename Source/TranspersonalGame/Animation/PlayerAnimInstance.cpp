#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion state defaults
    LocomotionState = EAnim_LocomotionState::Idle;
    GroundSpeed = 0.0f;
    MovementDirection = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsMoving = false;
    bIsSprinting = false;

    // Survival defaults
    FearLevel = 0.0f;
    StaminaLevel = 1.0f;
    bIsExhausted = false;

    // Foot IK config
    FootIKTraceDistance = 80.0f;
    FootIKInterpSpeed = 15.0f;
    PelvisAdjustmentSpeed = 10.0f;

    // Speed thresholds
    WalkSpeed = 150.0f;
    RunSpeed = 400.0f;
    SprintSpeed = 600.0f;
    MinMovingSpeed = 10.0f;

    // Internal
    PreviousVelocityX = 0.0f;
    PreviousVelocityY = 0.0f;
    LeanSmoothing = 8.0f;

    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

// ============================================================
// NativeInitializeAnimation
// ============================================================

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    if (Pawn)
    {
        OwnerCharacter = Cast<ACharacter>(Pawn);
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
    }
}

// ============================================================
// NativeUpdateAnimation — called every frame
// ============================================================

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        // Re-attempt cache if character not yet set
        APawn* Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            OwnerCharacter = Cast<ACharacter>(Pawn);
            if (OwnerCharacter)
            {
                MovementComponent = OwnerCharacter->GetCharacterMovement();
            }
        }
        return;
    }

    UpdateLocomotionData(DeltaSeconds);
    UpdateLocomotionState();
    UpdateLean(DeltaSeconds);
    UpdateFootIK(DeltaSeconds);
    UpdateSurvivalInfluence();
}

// ============================================================
// NativePostEvaluateAnimation
// ============================================================

void UPlayerAnimInstance::NativePostEvaluateAnimation()
{
    Super::NativePostEvaluateAnimation();
    // Post-evaluate: foot IK bone transforms applied here via AnimGraph nodes
    // The FootIKData struct is read by the AnimGraph Two Bone IK nodes
}

// ============================================================
// UpdateLocomotionData
// ============================================================

void UPlayerAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    FVector Velocity = MovementComponent->Velocity;
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);

    GroundSpeed = HorizontalVelocity.Size();
    bIsMoving = GroundSpeed > MinMovingSpeed;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    // Calculate movement direction relative to actor facing
    if (bIsMoving)
    {
        FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        FVector LocalVelocity = ActorRotation.UnrotateVector(HorizontalVelocity);
        MovementDirection = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
    }
    else
    {
        MovementDirection = 0.0f;
    }

    // Sprint detection — speed above run threshold
    bIsSprinting = GroundSpeed > RunSpeed;

    // Populate LocomotionData struct
    LocomotionData.Speed = GroundSpeed;
    LocomotionData.Direction = MovementDirection;
    LocomotionData.bIsMoving = bIsMoving;
    LocomotionData.bIsInAir = bIsInAir;
    LocomotionData.bIsCrouching = bIsCrouching;
    LocomotionData.bIsSprinting = bIsSprinting;

    // Aim offsets from controller rotation
    if (AController* Controller = OwnerCharacter->GetController())
    {
        FRotator ControlRotation = Controller->GetControlRotation();
        FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);
        LocomotionData.AimPitch = FMath::ClampAngle(DeltaRotation.Pitch, -90.0f, 90.0f);
        LocomotionData.AimYaw = FMath::ClampAngle(DeltaRotation.Yaw, -90.0f, 90.0f);
    }
}

// ============================================================
// UpdateLocomotionState — state machine logic
// ============================================================

void UPlayerAnimInstance::UpdateLocomotionState()
{
    EAnim_LocomotionState PreviousState = LocomotionState;

    if (bIsInAir)
    {
        FVector Velocity = MovementComponent ? MovementComponent->Velocity : FVector::ZeroVector;
        if (Velocity.Z < -100.0f)
        {
            LocomotionState = EAnim_LocomotionState::Fall;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Jump;
        }
    }
    else if (bIsCrouching)
    {
        LocomotionState = bIsMoving ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
    }
    else if (bIsMoving)
    {
        if (bIsSprinting || GroundSpeed > RunSpeed)
        {
            LocomotionState = EAnim_LocomotionState::Sprint;
        }
        else if (GroundSpeed > WalkSpeed)
        {
            LocomotionState = EAnim_LocomotionState::Run;
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Walk;
        }
    }
    else
    {
        // Landing transition — if we just landed, play land then go to idle
        if (PreviousState == EAnim_LocomotionState::Fall || PreviousState == EAnim_LocomotionState::Jump)
        {
            LocomotionState = EAnim_LocomotionState::Land;
        }
        else if (LocomotionState == EAnim_LocomotionState::Land)
        {
            // Land is a one-shot — transition handled by AnimGraph notify
            // Keep Land until AnimNotify fires to reset to Idle
        }
        else
        {
            LocomotionState = EAnim_LocomotionState::Idle;
        }
    }
}

// ============================================================
// UpdateFootIK — two-bone IK per foot with terrain adaptation
// ============================================================

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir)
    {
        // In air: blend IK out
        FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, 0.0f, DeltaSeconds, FootIKInterpSpeed);
        FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, 0.0f, DeltaSeconds, PelvisAdjustmentSpeed);
        return;
    }

    FVector TargetLeftLoc, TargetRightLoc;
    FRotator TargetLeftRot, TargetRightRot;

    bool bLeftHit = TraceFootPosition(FName("foot_l"), TargetLeftLoc, TargetLeftRot);
    bool bRightHit = TraceFootPosition(FName("foot_r"), TargetRightLoc, TargetRightRot);

    FootIKData.bLeftFootGrounded = bLeftHit;
    FootIKData.bRightFootGrounded = bRightHit;

    // Interpolate foot locations
    FootIKData.LeftFootLocation = FMath::VInterpTo(
        FootIKData.LeftFootLocation, TargetLeftLoc, DeltaSeconds, FootIKInterpSpeed);
    FootIKData.RightFootLocation = FMath::VInterpTo(
        FootIKData.RightFootLocation, TargetRightLoc, DeltaSeconds, FootIKInterpSpeed);

    // Interpolate foot rotations
    FootIKData.LeftFootRotation = FMath::RInterpTo(
        FootIKData.LeftFootRotation, TargetLeftRot, DeltaSeconds, FootIKInterpSpeed);
    FootIKData.RightFootRotation = FMath::RInterpTo(
        FootIKData.RightFootRotation, TargetRightRot, DeltaSeconds, FootIKInterpSpeed);

    // Alpha: full IK when grounded and not moving fast
    float TargetAlpha = (GroundSpeed < RunSpeed) ? 1.0f : FMath::GetMappedRangeValueClamped(
        FVector2D(RunSpeed, SprintSpeed), FVector2D(1.0f, 0.0f), GroundSpeed);

    FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, bLeftHit ? TargetAlpha : 0.0f, DeltaSeconds, FootIKInterpSpeed);
    FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, bRightHit ? TargetAlpha : 0.0f, DeltaSeconds, FootIKInterpSpeed);

    // Pelvis offset — lower pelvis to accommodate the lower foot
    float TargetPelvis = CalculatePelvisOffset();
    FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, TargetPelvis, DeltaSeconds, PelvisAdjustmentSpeed);
}

// ============================================================
// TraceFootPosition — line trace from foot socket downward
// ============================================================

bool UPlayerAnimInstance::TraceFootPosition(const FName& FootSocketName, FVector& OutLocation, FRotator& OutRotation)
{
    if (!OwnerCharacter)
    {
        return false;
    }

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        return false;
    }

    FVector SocketLocation = Mesh->GetSocketLocation(FootSocketName);
    FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, FootIKTraceDistance * 0.5f);
    FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    QueryParams.bTraceComplex = false;

    UWorld* CurrentWorld = GetWorld();
    if (!CurrentWorld)
    {
        return false;
    }

    bool bHit = CurrentWorld->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd,
        ECollisionChannel::ECC_Visibility, QueryParams);

    if (bHit)
    {
        OutLocation = HitResult.ImpactPoint;

        // Calculate foot rotation from surface normal
        FVector SurfaceNormal = HitResult.ImpactNormal;
        FRotator SurfaceRotation = UKismetMathLibrary::MakeRotFromZX(SurfaceNormal, OwnerCharacter->GetActorForwardVector());
        OutRotation = FRotator(SurfaceRotation.Pitch, 0.0f, SurfaceRotation.Roll);
    }

    return bHit;
}

// ============================================================
// CalculatePelvisOffset
// ============================================================

float UPlayerAnimInstance::CalculatePelvisOffset() const
{
    float LeftZ = FootIKData.LeftFootLocation.Z;
    float RightZ = FootIKData.RightFootLocation.Z;
    float MinZ = FMath::Min(LeftZ, RightZ);

    if (!OwnerCharacter)
    {
        return 0.0f;
    }

    float ActorZ = OwnerCharacter->GetActorLocation().Z;
    float Offset = MinZ - ActorZ;

    // Clamp pelvis offset to reasonable range
    return FMath::Clamp(Offset, -30.0f, 0.0f);
}

// ============================================================
// UpdateLean — velocity-based lean for realistic weight
// ============================================================

void UPlayerAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter || !MovementComponent || bIsInAir)
    {
        LeanData.LeanForwardBack = FMath::FInterpTo(LeanData.LeanForwardBack, 0.0f, DeltaSeconds, LeanSmoothing);
        LeanData.LeanLeftRight = FMath::FInterpTo(LeanData.LeanLeftRight, 0.0f, DeltaSeconds, LeanSmoothing);
        return;
    }

    FVector Velocity = MovementComponent->Velocity;
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    FVector LocalVelocity = ActorRotation.UnrotateVector(Velocity);

    // Acceleration-based lean (delta velocity)
    float AccelX = (LocalVelocity.X - PreviousVelocityX) / FMath::Max(DeltaSeconds, 0.001f);
    float AccelY = (LocalVelocity.Y - PreviousVelocityY) / FMath::Max(DeltaSeconds, 0.001f);

    PreviousVelocityX = LocalVelocity.X;
    PreviousVelocityY = LocalVelocity.Y;

    // Scale and clamp lean values
    float TargetLeanFB = FMath::Clamp(AccelX * 0.005f, -15.0f, 15.0f);
    float TargetLeanLR = FMath::Clamp(AccelY * 0.005f, -15.0f, 15.0f);

    // Exhaustion increases sway
    float ExhaustionMultiplier = bIsExhausted ? 1.5f : 1.0f;
    TargetLeanFB *= ExhaustionMultiplier;
    TargetLeanLR *= ExhaustionMultiplier;

    LeanData.LeanForwardBack = FMath::FInterpTo(LeanData.LeanForwardBack, TargetLeanFB, DeltaSeconds, LeanSmoothing);
    LeanData.LeanLeftRight = FMath::FInterpTo(LeanData.LeanLeftRight, TargetLeanLR, DeltaSeconds, LeanSmoothing);
}

// ============================================================
// UpdateSurvivalInfluence — survival stats affect animation
// ============================================================

void UPlayerAnimInstance::UpdateSurvivalInfluence()
{
    // Exhaustion: stamina below 20% triggers exhausted state
    bIsExhausted = StaminaLevel < 0.2f;

    // Fear: high fear increases movement speed multiplier in AnimGraph
    // (FearLevel is set externally by the character's survival component)
}

// ============================================================
// Blueprint callable implementations
// ============================================================

EAnim_LocomotionState UPlayerAnimInstance::GetLocomotionState() const
{
    return LocomotionState;
}

FAnim_FootIKData UPlayerAnimInstance::GetFootIKData() const
{
    return FootIKData;
}

FAnim_LeanData UPlayerAnimInstance::GetLeanData() const
{
    return LeanData;
}

bool UPlayerAnimInstance::IsInLocomotionState(EAnim_LocomotionState State) const
{
    return LocomotionState == State;
}
