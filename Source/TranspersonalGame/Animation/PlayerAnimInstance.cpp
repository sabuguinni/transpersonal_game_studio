// PlayerAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full implementation of UPlayerAnimInstance:
//   - Locomotion (speed, direction, lean, pitch tilt)
//   - Survival state mirroring from TranspersonalCharacter
//   - Foot IK via line traces
//   - Jump / land detection

#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
UPlayerAnimInstance::UPlayerAnimInstance()
{
	// Locomotion defaults
	Speed              = 0.f;
	SmoothedSpeed      = 0.f;
	Direction          = 0.f;
	bIsMoving          = false;
	bIsSprinting       = false;
	bIsCrouching       = false;
	bIsInAir           = false;
	VerticalVelocity   = 0.f;
	LeanAmount         = 0.f;
	PitchTilt          = 0.f;

	// Survival defaults
	bIsSneaking        = false;
	bIsClimbing        = false;
	FearLevel          = 0.f;
	StaminaLevel       = 1.f;
	HealthLevel        = 1.f;
	bIsCarrying        = false;
	bIsAiming          = false;

	// Foot IK defaults
	LeftFootIKOffset   = FVector::ZeroVector;
	RightFootIKOffset  = FVector::ZeroVector;
	LeftFootIKAlpha    = 0.f;
	RightFootIKAlpha   = 0.f;
	PelvisOffset       = 0.f;

	// Blend space defaults
	BlendSpaceX        = 0.f;
	BlendSpaceY        = 0.f;

	// Jump defaults
	TimeInAir          = 0.f;
	bJustLanded        = false;
	LandingImpactSpeed = 0.f;

	// Tuning defaults
	SpeedSmoothingRate    = 10.f;
	LeanSmoothingRate     = 8.f;
	FootIKTraceDistance   = 50.f;
	FootIKTraceUpDistance = 50.f;

	// Internal
	PreviousVelocity = FVector::ZeroVector;
	AirTime          = 0.f;
	LandedTimer      = 0.f;
	bWasInAir        = false;
	OwnerCharacter   = nullptr;
	MovementComponent = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeInitializeAnimation
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Pawn = TryGetPawnOwner();
	if (Pawn)
	{
		OwnerCharacter    = Cast<ACharacter>(Pawn);
		if (OwnerCharacter)
		{
			MovementComponent = OwnerCharacter->GetCharacterMovement();
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeUpdateAnimation — called every frame
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter || !MovementComponent)
	{
		// Try to re-cache if not set yet
		APawn* Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			OwnerCharacter    = Cast<ACharacter>(Pawn);
			if (OwnerCharacter)
				MovementComponent = OwnerCharacter->GetCharacterMovement();
		}
		return;
	}

	UpdateLocomotion(DeltaSeconds);
	UpdateSurvivalStates(DeltaSeconds);
	UpdateFootIK(DeltaSeconds);
	UpdateJumpState(DeltaSeconds);
}

// ─────────────────────────────────────────────────────────────────────────────
// NativePostEvaluateAnimation
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::NativePostEvaluateAnimation()
{
	Super::NativePostEvaluateAnimation();
	// Reserved for post-evaluate IK corrections if needed
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateLocomotion
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
	const FVector Velocity    = MovementComponent->Velocity;
	const FVector HorizVel    = FVector(Velocity.X, Velocity.Y, 0.f);
	const float   RawSpeed    = HorizVel.Size();

	Speed            = RawSpeed;
	VerticalVelocity = Velocity.Z;
	bIsMoving        = RawSpeed > 10.f;
	bIsInAir         = MovementComponent->IsFalling();
	bIsCrouching     = OwnerCharacter->bIsCrouched;

	// Smooth speed for blend space
	SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, RawSpeed, DeltaSeconds, SpeedSmoothingRate);

	// Direction angle relative to actor forward
	if (bIsMoving)
	{
		const FRotator ActorRot  = OwnerCharacter->GetActorRotation();
		const FVector  Forward   = ActorRot.Vector();
		Direction = UKismetMathLibrary::DegAtan2(
			FVector::DotProduct(HorizVel.GetSafeNormal(), ActorRot.RotateVector(FVector::RightVector)),
			FVector::DotProduct(HorizVel.GetSafeNormal(), Forward)
		);
	}
	else
	{
		Direction = 0.f;
	}

	// Sprint detection — speed > 350 cm/s (walk ~150, run ~300, sprint ~500)
	bIsSprinting = RawSpeed > 350.f;

	// Lean — based on lateral acceleration
	const FVector Accel        = (Velocity - PreviousVelocity) / FMath::Max(DeltaSeconds, KINDA_SMALL_NUMBER);
	const FVector LocalAccel   = OwnerCharacter->GetActorTransform().InverseTransformVector(Accel);
	const float   TargetLean   = FMath::Clamp(LocalAccel.Y / 600.f, -1.f, 1.f);
	LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, LeanSmoothingRate);

	// Pitch tilt for slopes
	const float SlopePitch = MovementComponent->GetCurrentFloor().HitResult.ImpactNormal.Z;
	const float TargetPitch = FMath::Clamp(1.f - SlopePitch, -1.f, 1.f);
	PitchTilt = FMath::FInterpTo(PitchTilt, TargetPitch, DeltaSeconds, 5.f);

	// Blend space inputs
	BlendSpaceX = Direction;
	BlendSpaceY = SmoothedSpeed;

	PreviousVelocity = Velocity;
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateSurvivalStates
// Mirror survival stats from the owning character if it exposes them.
// Uses a safe property-name approach so this compiles even if the
// TranspersonalCharacter properties change.
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateSurvivalStates(float DeltaSeconds)
{
	if (!OwnerCharacter) return;

	// Try to read survival stats via UObject property reflection
	// This avoids a hard dependency on TranspersonalCharacter's exact header
	UClass* CharClass = OwnerCharacter->GetClass();

	auto ReadFloat = [&](const FName PropName, float DefaultVal) -> float
	{
		FFloatProperty* Prop = FindFProperty<FFloatProperty>(CharClass, PropName);
		if (Prop)
			return Prop->GetPropertyValue_InContainer(OwnerCharacter);
		return DefaultVal;
	};

	auto ReadBool = [&](const FName PropName, bool DefaultVal) -> bool
	{
		FBoolProperty* Prop = FindFProperty<FBoolProperty>(CharClass, PropName);
		if (Prop)
			return Prop->GetPropertyValue_InContainer(OwnerCharacter);
		return DefaultVal;
	};

	// Mirror survival stats (property names from TranspersonalCharacter)
	HealthLevel   = ReadFloat(TEXT("Health"),   1.f) / FMath::Max(ReadFloat(TEXT("MaxHealth"),   100.f), 1.f);
	StaminaLevel  = ReadFloat(TEXT("Stamina"),  1.f) / FMath::Max(ReadFloat(TEXT("MaxStamina"),  100.f), 1.f);
	FearLevel     = FMath::Clamp(ReadFloat(TEXT("Fear"), 0.f) / 100.f, 0.f, 1.f);

	bIsSneaking   = ReadBool(TEXT("bIsSneaking"),   false);
	bIsClimbing   = ReadBool(TEXT("bIsClimbing"),   false);
	bIsCarrying   = ReadBool(TEXT("bIsCarrying"),   false);
	bIsAiming     = ReadBool(TEXT("bIsAiming"),     false);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateFootIK
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	// Only apply foot IK when on ground and not crouching
	if (bIsInAir || bIsCrouching)
	{
		LeftFootIKAlpha  = FMath::FInterpTo(LeftFootIKAlpha,  0.f, DeltaSeconds, 10.f);
		RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.f, DeltaSeconds, 10.f);
		PelvisOffset     = FMath::FInterpTo(PelvisOffset,     0.f, DeltaSeconds, 10.f);
		return;
	}

	SolveFootIK(TEXT("foot_l"), LeftFootIKOffset,  LeftFootIKAlpha,  DeltaSeconds);
	SolveFootIK(TEXT("foot_r"), RightFootIKOffset, RightFootIKAlpha, DeltaSeconds);

	// Pelvis offset = lowest foot offset to prevent foot clipping through ground
	const float LowestFoot = FMath::Min(LeftFootIKOffset.Z, RightFootIKOffset.Z);
	PelvisOffset = FMath::FInterpTo(PelvisOffset, LowestFoot, DeltaSeconds, 15.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// SolveFootIK — line trace from foot bone to find ground contact
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::SolveFootIK(FName FootBoneName, FVector& OutIKOffset, float& OutIKAlpha, float DeltaSeconds)
{
	if (!OwnerCharacter) return;

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return;

	const FVector FootWorldPos = Mesh->GetSocketLocation(FootBoneName);
	const FVector TraceStart   = FootWorldPos + FVector(0.f, 0.f, FootIKTraceUpDistance);
	const FVector TraceEnd     = FootWorldPos - FVector(0.f, 0.f, FootIKTraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd,
		ECC_Visibility, Params
	);

	if (bHit)
	{
		// Offset in component space
		const FVector HitLocal = Mesh->GetComponentTransform().InverseTransformPosition(HitResult.ImpactPoint);
		const FVector FootLocal = Mesh->GetComponentTransform().InverseTransformPosition(FootWorldPos);
		const FVector TargetOffset = FVector(0.f, 0.f, HitLocal.Z - FootLocal.Z);

		OutIKOffset = FMath::VInterpTo(OutIKOffset, TargetOffset, DeltaSeconds, 15.f);
		OutIKAlpha  = FMath::FInterpTo(OutIKAlpha,  1.f,          DeltaSeconds, 10.f);
	}
	else
	{
		OutIKOffset = FMath::VInterpTo(OutIKOffset, FVector::ZeroVector, DeltaSeconds, 10.f);
		OutIKAlpha  = FMath::FInterpTo(OutIKAlpha,  0.f,                 DeltaSeconds, 10.f);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateJumpState
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateJumpState(float DeltaSeconds)
{
	const bool bCurrentlyInAir = MovementComponent->IsFalling();

	if (bCurrentlyInAir)
	{
		AirTime += DeltaSeconds;
		bJustLanded = false;
	}
	else
	{
		// Landing detection
		if (bWasInAir && !bCurrentlyInAir)
		{
			bJustLanded        = true;
			LandingImpactSpeed = FMath::Abs(VerticalVelocity);
			LandedTimer        = 0.35f;  // Hold landed state for 350ms
		}

		if (LandedTimer > 0.f)
		{
			LandedTimer -= DeltaSeconds;
			if (LandedTimer <= 0.f)
			{
				bJustLanded = false;
				LandedTimer = 0.f;
			}
		}

		AirTime = 0.f;
	}

	TimeInAir  = AirTime;
	bWasInAir  = bCurrentlyInAir;
}
