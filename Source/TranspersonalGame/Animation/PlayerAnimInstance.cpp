// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260630_002
// Implements UAnimInstance subclass for the prehistoric survivor player character.
// Drives locomotion state machine, IK foot placement, survival state blending.

#include "PlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Locomotion defaults
    Speed = 0.0f;
    SmoothedSpeed = 0.0f;
    Direction = 0.0f;
    LeanAmount = 0.0f;
    VerticalVelocity = 0.0f;

    bIsMoving = false;
    bIsSprinting = false;
    bIsCrouching = false;
    bIsInAir = false;
    bIsSneaking = false;
    bIsClimbing = false;

    // Survival state defaults
    FearLevel = 0.0f;
    StaminaLevel = 1.0f;
    HungerLevel = 1.0f;
    ThirstLevel = 1.0f;

    // IK defaults
    bEnableFootIK = true;
    LeftFootIKOffset = FVector::ZeroVector;
    RightFootIKOffset = FVector::ZeroVector;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    IKTraceDistance = 55.0f;
    IKInterpSpeed = 15.0f;

    // State machine defaults
    LocomotionState = EAnim_LocomotionState::Idle;
    PreviousLocomotionState = EAnim_LocomotionState::Idle;

    // Blending
    SpeedBlendWeight = 0.0f;
    DirectionBlendWeight = 0.0f;
    AimPitch = 0.0f;
    AimYaw = 0.0f;

    // Internal
    OwnerCharacter = nullptr;
    OwnerMovement = nullptr;
    LastVelocity = FVector::ZeroVector;
    LeanVelocity = FVector::ZeroVector;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn) return;

    OwnerCharacter = Cast<ACharacter>(OwnerPawn);
    if (OwnerCharacter)
    {
        OwnerMovement = OwnerCharacter->GetCharacterMovement();
    }
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !OwnerMovement || DeltaSeconds <= 0.0f) return;

    UpdateLocomotionData(DeltaSeconds);
    UpdateAimData();
    UpdateSurvivalStates();
    UpdateFootIK(DeltaSeconds);
    UpdateLocomotionState();
    UpdateLean(DeltaSeconds);
}

void UPlayerAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
    FVector Velocity = OwnerCharacter->GetVelocity();
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);

    Speed = HorizontalVelocity.Size();
    VerticalVelocity = Velocity.Z;

    // Smooth speed for blend space transitions
    SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, 8.0f);

    // Direction relative to actor forward
    if (Speed > 1.0f)
    {
        FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        FVector LocalVelocity = ActorRotation.UnrotateVector(HorizontalVelocity);
        Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
    }
    else
    {
        Direction = FMath::FInterpTo(Direction, 0.0f, DeltaSeconds, 5.0f);
    }

    // Movement state flags
    bIsMoving = Speed > 3.0f;
    bIsInAir = OwnerMovement->IsFalling();
    bIsCrouching = OwnerMovement->IsCrouching();
    bIsSprinting = Speed > 400.0f && bIsMoving && !bIsCrouching;

    // Speed blend weight (0=idle, 1=walk, 2=run, 3=sprint)
    if (Speed < 10.0f)
        SpeedBlendWeight = 0.0f;
    else if (Speed < 200.0f)
        SpeedBlendWeight = FMath::GetMappedRangeValueClamped(FVector2D(10.0f, 200.0f), FVector2D(0.0f, 1.0f), Speed);
    else if (Speed < 400.0f)
        SpeedBlendWeight = FMath::GetMappedRangeValueClamped(FVector2D(200.0f, 400.0f), FVector2D(1.0f, 2.0f), Speed);
    else
        SpeedBlendWeight = FMath::GetMappedRangeValueClamped(FVector2D(400.0f, 600.0f), FVector2D(2.0f, 3.0f), Speed);

    DirectionBlendWeight = Direction / 180.0f;
}

void UPlayerAnimInstance::UpdateAimData()
{
    if (!OwnerCharacter) return;

    FRotator ActorRot = OwnerCharacter->GetActorRotation();
    FRotator ControlRot = OwnerCharacter->GetControlRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.0f, 90.0f);
}

void UPlayerAnimInstance::UpdateSurvivalStates()
{
    // FearLevel affects animation speed and posture blending
    // These values are set externally by the character's survival component
    // Clamp all values to valid range
    FearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    StaminaLevel = FMath::Clamp(StaminaLevel, 0.0f, 1.0f);
    HungerLevel = FMath::Clamp(HungerLevel, 0.0f, 1.0f);
    ThirstLevel = FMath::Clamp(ThirstLevel, 0.0f, 1.0f);

    // Exhaustion affects movement — low stamina forces walk
    if (StaminaLevel < 0.15f && bIsSprinting)
    {
        bIsSprinting = false;
    }
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
    PreviousLocomotionState = LocomotionState;

    if (bIsClimbing)
    {
        LocomotionState = EAnim_LocomotionState::Climbing;
    }
    else if (bIsInAir)
    {
        LocomotionState = VerticalVelocity > 0.0f
            ? EAnim_LocomotionState::Jump
            : EAnim_LocomotionState::Fall;
    }
    else if (bIsCrouching)
    {
        LocomotionState = bIsMoving
            ? EAnim_LocomotionState::CrouchWalk
            : EAnim_LocomotionState::Crouch;
    }
    else if (bIsSneaking)
    {
        LocomotionState = EAnim_LocomotionState::Sneak;
    }
    else if (bIsSprinting)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (bIsMoving)
    {
        LocomotionState = Speed > 200.0f
            ? EAnim_LocomotionState::Run
            : EAnim_LocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
}

void UPlayerAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter) return;

    FVector CurrentVelocity = OwnerCharacter->GetVelocity();
    FVector VelocityDelta = CurrentVelocity - LastVelocity;
    LastVelocity = CurrentVelocity;

    // Lateral lean based on acceleration
    FVector LocalDelta = OwnerCharacter->GetActorRotation().UnrotateVector(VelocityDelta);
    float TargetLean = FMath::Clamp(LocalDelta.Y * 0.02f, -1.0f, 1.0f);
    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, 4.0f);
}

void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!bEnableFootIK || !OwnerCharacter) return;

    // Trace for left foot
    FVector LeftFootLocation = OwnerCharacter->GetMesh()
        ? OwnerCharacter->GetMesh()->GetSocketLocation(FName("foot_l"))
        : OwnerCharacter->GetActorLocation();

    // Trace for right foot
    FVector RightFootLocation = OwnerCharacter->GetMesh()
        ? OwnerCharacter->GetMesh()->GetSocketLocation(FName("foot_r"))
        : OwnerCharacter->GetActorLocation();

    FVector TargetLeftOffset = FVector::ZeroVector;
    FVector TargetRightOffset = FVector::ZeroVector;
    FRotator TargetLeftRot = FRotator::ZeroRotator;
    FRotator TargetRightRot = FRotator::ZeroRotator;

    // Only do IK when on ground
    if (!bIsInAir)
    {
        TargetLeftOffset = TraceFootIK(LeftFootLocation, TargetLeftRot);
        TargetRightOffset = TraceFootIK(RightFootLocation, TargetRightRot);
    }

    // Smooth IK offsets
    LeftFootIKOffset = FMath::VInterpTo(LeftFootIKOffset, TargetLeftOffset, DeltaSeconds, IKInterpSpeed);
    RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, TargetRightOffset, DeltaSeconds, IKInterpSpeed);
    LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, TargetLeftRot, DeltaSeconds, IKInterpSpeed);
    RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, TargetRightRot, DeltaSeconds, IKInterpSpeed);
}

FVector UPlayerAnimInstance::TraceFootIK(const FVector& FootWorldLocation, FRotator& OutFootRotation)
{
    if (!OwnerCharacter) return FVector::ZeroVector;

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World) return FVector::ZeroVector;

    FVector TraceStart = FootWorldLocation + FVector(0.0f, 0.0f, IKTraceDistance);
    FVector TraceEnd = FootWorldLocation - FVector(0.0f, 0.0f, IKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Calculate foot rotation from surface normal
        FVector SurfaceNormal = HitResult.Normal;
        FRotator SurfaceRotation = UKismetMathLibrary::MakeRotFromZX(SurfaceNormal, OwnerCharacter->GetActorForwardVector());
        OutFootRotation = FRotator(SurfaceRotation.Pitch, 0.0f, SurfaceRotation.Roll);

        // Return Z offset to place foot on surface
        float ZOffset = HitResult.Location.Z - FootWorldLocation.Z;
        return FVector(0.0f, 0.0f, ZOffset);
    }

    OutFootRotation = FRotator::ZeroRotator;
    return FVector::ZeroVector;
}

void UPlayerAnimInstance::SetFearLevel(float NewFearLevel)
{
    FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
}

void UPlayerAnimInstance::SetStaminaLevel(float NewStaminaLevel)
{
    StaminaLevel = FMath::Clamp(NewStaminaLevel, 0.0f, 1.0f);
}

void UPlayerAnimInstance::SetSneaking(bool bNewSneaking)
{
    bIsSneaking = bNewSneaking;
}

void UPlayerAnimInstance::SetClimbing(bool bNewClimbing)
{
    bIsClimbing = bNewClimbing;
}

EAnim_LocomotionState UPlayerAnimInstance::GetLocomotionState() const
{
    return LocomotionState;
}

bool UPlayerAnimInstance::WasLocomotionStateChangedThisFrame() const
{
    return LocomotionState != PreviousLocomotionState;
}
