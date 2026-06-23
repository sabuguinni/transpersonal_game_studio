#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
    // Defaults set in header via member initializers
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    UpdateLocomotionState();
    UpdateCombatState();
    UpdateSurvivalState();
    UpdateFootIK(DeltaSeconds);
}

void UDinoSurvivorAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // Thread-safe property reads can go here for performance-critical paths
}

// ─── Locomotion ─────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotionState()
{
    if (!OwnerCharacter || !MovementComponent) return;

    const FVector Velocity = MovementComponent->Velocity;
    GroundSpeed = Velocity.Size2D();
    bIsMoving = GroundSpeed > 3.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = OwnerCharacter->bIsCrouched;

    // Determine movement direction relative to character facing
    const FRotator ActorRot = OwnerCharacter->GetActorRotation();
    const FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
    LocomotionData.Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
    LocomotionData.Speed = GroundSpeed;
    LocomotionData.bIsInAir = bIsInAir;
    LocomotionData.bIsCrouching = bIsCrouching;
    LocomotionData.bIsSprinting = bIsSprinting;

    // Aim offset
    const FRotator ControlRot = OwnerCharacter->GetControlRotation();
    const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
    AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
    AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.0f, 90.0f);
    LocomotionData.AimPitch = AimPitch;
    LocomotionData.AimYaw = AimYaw;

    // Lean — proportional to lateral velocity
    const float TargetLean = FMath::Clamp(LocalVelocity.Y / 300.0f, -1.0f, 1.0f);
    LocomotionData.LeanAmount = FMath::FInterpTo(LocomotionData.LeanAmount, TargetLean, GetWorld()->GetDeltaSeconds(), 6.0f);

    // Determine state
    if (bIsInAir)
    {
        MovementState = Velocity.Z > 0.0f ? EAnim_MovementState::Jumping : EAnim_MovementState::Falling;
    }
    else if (bIsCrouching)
    {
        MovementState = EAnim_MovementState::Crouching;
    }
    else if (bIsSprinting && bIsMoving)
    {
        MovementState = EAnim_MovementState::Sprinting;
    }
    else if (GroundSpeed > 250.0f && bIsMoving)
    {
        MovementState = EAnim_MovementState::Running;
    }
    else if (bIsMoving)
    {
        MovementState = EAnim_MovementState::Walking;
    }
    else
    {
        MovementState = EAnim_MovementState::Idle;
    }
}

// ─── Combat ─────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateCombatState()
{
    // Combat state is set externally via SetCombatState().
    // Here we update aim offsets for armed stances.
    if (CombatState != EAnim_CombatState::Unarmed)
    {
        // Clamp aim yaw for weapon tracking
        AimYaw = FMath::ClampAngle(AimYaw, -60.0f, 60.0f);
    }
}

void UDinoSurvivorAnimInstance::SetCombatState(EAnim_CombatState NewState)
{
    CombatState = NewState;
}

void UDinoSurvivorAnimInstance::TriggerAttackMontage()
{
    bIsAttacking = true;
    // Montage playback is driven from Blueprint using Montage_Play()
    // Reset flag via anim notify in the montage
}

void UDinoSurvivorAnimInstance::TriggerDeathMontage()
{
    MovementState = EAnim_MovementState::Idle;
    // Blueprint handles Montage_Play for death
}

void UDinoSurvivorAnimInstance::TriggerHitReactMontage()
{
    // Blueprint handles Montage_Play for hit react
}

// ─── Survival ───────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalState()
{
    bIsExhausted = StaminaNormalized < 0.15f;
    bIsInjured = HealthNormalized < 0.35f;
}

bool UDinoSurvivorAnimInstance::ShouldPlayExhaustionAnim() const
{
    return bIsExhausted && !bIsInAir;
}

float UDinoSurvivorAnimInstance::GetLeanAmount() const
{
    return LocomotionData.LeanAmount;
}

// ─── Foot IK ────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!IKData.bEnableFootIK || !OwnerCharacter) return;

    SolveFootIK(FName("foot_l"), IKData.LeftFootLocation, IKData.LeftFootRotation);
    SolveFootIK(FName("foot_r"), IKData.RightFootLocation, IKData.RightFootRotation);

    // Smooth pelvis offset
    const float MinFootZ = FMath::Min(IKData.LeftFootLocation.Z, IKData.RightFootLocation.Z);
    PelvisOffsetTarget = MinFootZ < 0.0f ? MinFootZ : 0.0f;
    CurrentPelvisOffset = FMath::FInterpTo(CurrentPelvisOffset, PelvisOffsetTarget, DeltaSeconds, PelvisAdjustSpeed);
    IKData.PelvisOffset = CurrentPelvisOffset;
}

void UDinoSurvivorAnimInstance::SolveFootIK(const FName& FootSocketName, FVector& OutLocation, FRotator& OutRotation)
{
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh) return;

    const FVector SocketLocation = Mesh->GetSocketLocation(FootSocketName);
    const FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
    const FVector TraceEnd = SocketLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        // Offset relative to character root
        const FVector CharLocation = OwnerCharacter->GetActorLocation();
        OutLocation = FVector(0.0f, 0.0f, HitResult.Location.Z - CharLocation.Z);

        // Foot rotation from surface normal
        const FVector Forward = FVector::CrossProduct(OwnerCharacter->GetActorRightVector(), HitResult.Normal);
        OutRotation = FRotationMatrix::MakeFromZX(HitResult.Normal, Forward).Rotator();
    }
    else
    {
        OutLocation = FVector::ZeroVector;
        OutRotation = FRotator::ZeroRotator;
    }
}
