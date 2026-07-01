#include "PlayerAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
    // Default thresholds — tuned for prehistoric survival pacing
    WalkThreshold   = 10.0f;
    RunThreshold    = 200.0f;
    SprintThreshold = 500.0f;
    LeanInterpSpeed = 4.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeInitializeAnimation
// Called once when the AnimInstance is created and linked to a skeletal mesh.
// Cache character and movement component references here — never in tick.
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // GetOwningActor() returns the actor that owns the SkeletalMeshComponent
    AActor* Owner = GetOwningActor();
    if (!Owner)
    {
        return;
    }

    OwnerCharacter = Cast<ACharacter>(Owner);
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeUpdateAnimation
// Called every frame. Update all anim properties that Blueprint reads.
// Keep this lean — no heavy computation, no allocations.
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        // Re-attempt cache if refs were lost (e.g., after hot reload)
        NativeInitializeAnimation();
        return;
    }

    // ── Velocity & Speed ────────────────────────────────────────────────────
    const FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size2D(); // Horizontal speed only — vertical handled separately
    VerticalVelocity = Velocity.Z;

    // ── Direction (for strafe blend space) ──────────────────────────────────
    if (Speed > WalkThreshold)
    {
        const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
        const FRotator VelocityRotation = Velocity.Rotation();
        Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, ActorRotation).Yaw;
    }
    else
    {
        Direction = 0.0f;
    }

    // ── Air State ────────────────────────────────────────────────────────────
    const bool bWasInAir = bIsInAir;
    bIsInAir = MovementComponent->IsFalling();

    if (bIsInAir)
    {
        TimeInAir += DeltaSeconds;
    }
    else
    {
        if (bWasInAir && TimeInAir > 0.1f)
        {
            // Just landed — state machine will pick up Landing state
        }
        TimeInAir = 0.0f;
    }

    // ── Crouch State ─────────────────────────────────────────────────────────
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // ── Sprint State ─────────────────────────────────────────────────────────
    bIsSprinting = (Speed >= SprintThreshold);

    // ── Locomotion State Machine ─────────────────────────────────────────────
    UpdateLocomotionState();

    // ── Aim Offset ───────────────────────────────────────────────────────────
    UpdateAimOffset();

    // ── Lean ─────────────────────────────────────────────────────────────────
    UpdateLean(DeltaSeconds);

    // ── Foot IK active when grounded ─────────────────────────────────────────
    bFootIKActive = !bIsInAir && !bIsDead;

    // Store previous values for next frame
    PreviousSpeed = Speed;
    PreviousYaw   = OwnerCharacter->GetActorRotation().Yaw;
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateLocomotionState
// Determines the current state for the state machine.
// Priority: Dead > InAir > Landing > Crouching > Sprint > Run > Walk > Idle
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateLocomotionState()
{
    if (bIsDead)
    {
        LocomotionState = EAnim_LocomotionState::Dead;
        return;
    }

    if (bIsInAir)
    {
        LocomotionState = EAnim_LocomotionState::InAir;
        return;
    }

    // Landing: just touched ground after meaningful air time
    if (LocomotionState == EAnim_LocomotionState::InAir && !bIsInAir)
    {
        LocomotionState = EAnim_LocomotionState::Landing;
        return;
    }

    if (bIsCrouching)
    {
        LocomotionState = EAnim_LocomotionState::Crouching;
        return;
    }

    if (Speed >= SprintThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Sprint;
    }
    else if (Speed >= RunThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Run;
    }
    else if (Speed >= WalkThreshold)
    {
        LocomotionState = EAnim_LocomotionState::Walk;
    }
    else
    {
        LocomotionState = EAnim_LocomotionState::Idle;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateAimOffset
// Computes aim pitch/yaw from the controller rotation delta vs actor rotation.
// Clamped to [-90, 90] for blend space compatibility.
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateAimOffset()
{
    if (!OwnerCharacter)
    {
        return;
    }

    AController* Controller = OwnerCharacter->GetController();
    if (!Controller)
    {
        AimPitch = 0.0f;
        AimYaw   = 0.0f;
        return;
    }

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator ActorRotation   = OwnerCharacter->GetActorRotation();
    const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

    AimPitch = FMath::Clamp(Delta.Pitch, -90.0f, 90.0f);
    AimYaw   = FMath::Clamp(Delta.Yaw,   -90.0f, 90.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateLean
// Computes lean from yaw rate. Smoothly interpolated to avoid pop.
// Positive = leaning right (turning right), Negative = leaning left.
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateLean(float DeltaSeconds)
{
    if (!OwnerCharacter || DeltaSeconds <= 0.0f)
    {
        return;
    }

    const float CurrentYaw = OwnerCharacter->GetActorRotation().Yaw;
    const float YawDelta   = FMath::FindDeltaAngleDegrees(PreviousYaw, CurrentYaw);
    const float YawRate    = (DeltaSeconds > 0.0f) ? (YawDelta / DeltaSeconds) : 0.0f;

    // Normalise to [-1, 1] range (max lean at 180 deg/s turn rate)
    const float TargetLean = FMath::Clamp(YawRate / 180.0f, -1.0f, 1.0f);
    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, LeanInterpSpeed);
}

// ─────────────────────────────────────────────────────────────────────────────
// IsPlayingMontageInSlot
// Returns true if any montage is currently playing in the specified slot.
// ─────────────────────────────────────────────────────────────────────────────
bool UPlayerAnimInstance::IsPlayingMontageInSlot(EAnim_MontageSlot Slot) const
{
    FName SlotName = NAME_None;
    switch (Slot)
    {
        case EAnim_MontageSlot::DefaultSlot: SlotName = FName("DefaultSlot");  break;
        case EAnim_MontageSlot::UpperBody:   SlotName = FName("UpperBody");    break;
        case EAnim_MontageSlot::FullBody:    SlotName = FName("FullBody");     break;
        case EAnim_MontageSlot::Additive:    SlotName = FName("Additive");     break;
        default: break;
    }

    if (SlotName == NAME_None)
    {
        return false;
    }

    return IsAnyMontagePlaying() && IsSlotActive(SlotName);
}

// ─────────────────────────────────────────────────────────────────────────────
// GetLocomotionStateString
// Debug helper — returns the current state as a readable string.
// ─────────────────────────────────────────────────────────────────────────────
FString UPlayerAnimInstance::GetLocomotionStateString() const
{
    switch (LocomotionState)
    {
        case EAnim_LocomotionState::Idle:       return TEXT("Idle");
        case EAnim_LocomotionState::Walk:       return TEXT("Walk");
        case EAnim_LocomotionState::Run:        return TEXT("Run");
        case EAnim_LocomotionState::Sprint:     return TEXT("Sprint");
        case EAnim_LocomotionState::InAir:      return TEXT("InAir");
        case EAnim_LocomotionState::Landing:    return TEXT("Landing");
        case EAnim_LocomotionState::Crouching:  return TEXT("Crouching");
        case EAnim_LocomotionState::Dead:       return TEXT("Dead");
        default:                                return TEXT("Unknown");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SetFootIKState
// Called by FootIKComponent each tick to push updated IK data into the AnimInstance.
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::SetFootIKState(const FAnim_FootIKState& NewState)
{
    FootIKState = NewState;
}
