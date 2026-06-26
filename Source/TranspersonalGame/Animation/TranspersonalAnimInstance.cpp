#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
	MovementState = EAnim_MovementState::Idle;
	StanceState   = EAnim_StanceState::Upright;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter    = Cast<ACharacter>(GetOwningActor());
	MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter || !MovementComponent)
	{
		OwnerCharacter    = Cast<ACharacter>(GetOwningActor());
		MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
		return;
	}

	UpdateLocomotionData();
	UpdateIKData();
	UpdateSurvivalData();
	UpdateMovementState();
}

void UTranspersonalAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	// Thread-safe reads only — no actor modifications here
}

// ─────────────────────────────────────────────────────────────────────────────
// Locomotion
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateLocomotionData()
{
	if (!OwnerCharacter || !MovementComponent) return;

	const FVector Velocity       = MovementComponent->Velocity;
	const FVector HorizontalVel  = FVector(Velocity.X, Velocity.Y, 0.f);
	GroundSpeed                  = HorizontalVel.Size();

	bShouldMove = GroundSpeed > 3.f && MovementComponent->GetCurrentAcceleration().Size() > 0.f;
	bIsFalling  = MovementComponent->IsFalling();

	// Direction relative to character facing
	const FRotator ActorRot = OwnerCharacter->GetActorRotation();
	LocomotionData.Speed     = GroundSpeed;
	LocomotionData.Direction = UKismetMathLibrary::NormalizedDeltaRotator(
		UKismetMathLibrary::MakeRotFromX(HorizontalVel), ActorRot).Yaw;
	LocomotionData.bIsInAir    = bIsFalling;
	LocomotionData.bIsCrouching = MovementComponent->IsCrouching();
	LocomotionData.bIsAccelerating = MovementComponent->GetCurrentAcceleration().Size() > 0.f;

	// Walk/run blend alpha (0=walk, 1=run)
	const float WalkSpeed = MovementComponent->MaxWalkSpeedCrouched;
	const float RunSpeed  = MovementComponent->MaxWalkSpeed;
	WalkRunAlpha = FMath::Clamp((GroundSpeed - WalkSpeed) / FMath::Max(RunSpeed - WalkSpeed, 1.f), 0.f, 1.f);

	// Aim offset
	const FRotator AimRot = OwnerCharacter->GetBaseAimRotation();
	const FRotator Delta  = UKismetMathLibrary::NormalizedDeltaRotator(AimRot, ActorRot);
	AimYaw   = FMath::Clamp(Delta.Yaw,   -90.f, 90.f);
	AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);

	// Lean (yaw delta per frame)
	const float CurrentYaw = ActorRot.Yaw;
	const float YawDelta   = UKismetMathLibrary::NormalizedDeltaRotator(
		FRotator(0.f, CurrentYaw, 0.f), FRotator(0.f, PreviousYaw, 0.f)).Yaw;
	LocomotionData.LeanAmount = FMath::FInterpTo(LocomotionData.LeanAmount, YawDelta, GetWorld()->GetDeltaSeconds(), LeanInterpSpeed);
	PreviousYaw = CurrentYaw;

	// Stance
	StanceState = LocomotionData.bIsCrouching ? EAnim_StanceState::Crouched : EAnim_StanceState::Upright;
}

void UTranspersonalAnimInstance::UpdateMovementState()
{
	if (!MovementComponent) return;

	if (bIsFalling)
	{
		MovementState = MovementComponent->Velocity.Z > 0.f
			? EAnim_MovementState::Jump
			: EAnim_MovementState::Fall;
		return;
	}

	if (LocomotionData.bIsCrouching)
	{
		MovementState = GroundSpeed > 1.f ? EAnim_MovementState::Walk : EAnim_MovementState::Crouch;
		return;
	}

	if (GroundSpeed < 3.f)
	{
		MovementState = EAnim_MovementState::Idle;
	}
	else if (GroundSpeed < 200.f)
	{
		MovementState = EAnim_MovementState::Walk;
	}
	else if (GroundSpeed < 450.f)
	{
		MovementState = EAnim_MovementState::Run;
	}
	else
	{
		MovementState = EAnim_MovementState::Sprint;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Foot IK
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateIKData()
{
	if (!OwnerCharacter) return;

	PerformFootIKTrace(true,  IKData.LeftFootLocation,  IKData.LeftFootRotation,  IKData.LeftFootAlpha);
	PerformFootIKTrace(false, IKData.RightFootLocation, IKData.RightFootRotation, IKData.RightFootAlpha);

	// Pelvis offset: push pelvis down by the larger foot offset so both feet stay grounded
	const float MaxFootOffset = FMath::Min(IKData.LeftFootLocation.Z, IKData.RightFootLocation.Z);
	IKData.PelvisOffset = FMath::FInterpTo(IKData.PelvisOffset, MaxFootOffset, GetWorld()->GetDeltaSeconds(), PelvisInterpSpeed);
}

void UTranspersonalAnimInstance::PerformFootIKTrace(bool bIsLeftFoot, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
{
	if (!OwnerCharacter) return;

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return;

	const FName SocketName = bIsLeftFoot ? FName("foot_l") : FName("foot_r");
	const FVector SocketLocation = Mesh->GetSocketLocation(SocketName);

	// Trace downward from foot socket
	const FVector TraceStart = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z + IKTraceDistance);
	const FVector TraceEnd   = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z - IKTraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

	if (bHit)
	{
		// Foot offset from capsule base
		const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()
			? OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
			: 88.f;
		const float FootOffset = HitResult.Location.Z - (OwnerCharacter->GetActorLocation().Z - CapsuleHalfHeight);

		OutLocation = FMath::VInterpTo(OutLocation, FVector(0.f, 0.f, FootOffset), GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
		OutRotation = FMath::RInterpTo(OutRotation, FRotator(FMath::RadiansToDegrees(-FMath::Atan2(HitResult.Normal.X, HitResult.Normal.Z)),
			0.f, FMath::RadiansToDegrees(FMath::Atan2(HitResult.Normal.Y, HitResult.Normal.Z))), GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
		OutAlpha = FMath::FInterpTo(OutAlpha, 1.f, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
	}
	else
	{
		OutLocation = FMath::VInterpTo(OutLocation, FVector::ZeroVector, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
		OutRotation = FMath::RInterpTo(OutRotation, FRotator::ZeroRotator, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
		OutAlpha    = FMath::FInterpTo(OutAlpha, 0.f, GetWorld()->GetDeltaSeconds(), IKInterpSpeed);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival
// ─────────────────────────────────────────────────────────────────────────────

void UTranspersonalAnimInstance::UpdateSurvivalData()
{
	// Survival stats are read from the owning character's properties via Blueprint
	// or set externally by the Character class each tick.
	// Defaults are safe (full health/stamina, no fear).
	SurvivalData.bIsExhausted = SurvivalData.StaminaNormalized < 0.1f;
	SurvivalData.bIsInjured   = SurvivalData.HealthNormalized  < 0.3f;
}
