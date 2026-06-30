#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
	: Speed(0.f)
	, SmoothedSpeed(0.f)
	, Direction(0.f)
	, LeanAmount(0.f)
	, VerticalVelocity(0.f)
	, bIsMoving(false)
	, bIsSprinting(false)
	, bIsCrouching(false)
	, bIsInAir(false)
	, bIsSneaking(false)
	, bIsClimbing(false)
	, bJustLanded(false)
	, LocomotionState(EAnim_LocomotionState::Idle)
	, HealthNormalized(1.f)
	, StaminaNormalized(1.f)
	, FearLevel(0.f)
	, bIsExhausted(false)
	, bIsInjured(false)
	, WeaponState(EAnim_WeaponState::Unarmed)
	, bIsAiming(false)
	, bIsAttacking(false)
	, AimPitch(0.f)
	, AimYaw(0.f)
	, LeftFootIKLocation(FVector::ZeroVector)
	, RightFootIKLocation(FVector::ZeroVector)
	, LeftFootIKRotation(FRotator::ZeroRotator)
	, RightFootIKRotation(FRotator::ZeroRotator)
	, PelvisOffset(0.f)
	, bEnableFootIK(true)
	, AdditiveExhaustionWeight(0.f)
	, AdditiveFearWeight(0.f)
	, AdditiveInjuryWeight(0.f)
	, SpeedSmoothingRate(8.f)
	, LeanSmoothingRate(5.f)
	, FootIKTraceLength(50.f)
	, FootIKInterpSpeed(15.f)
	, WalkSpeedThreshold(10.f)
	, RunSpeedThreshold(250.f)
	, SprintSpeedThreshold(450.f)
	, OwnerCharacter(nullptr)
	, PreviousVelocity(FVector::ZeroVector)
	, LandingTimer(0.f)
	, LeftFootIKTarget(FVector::ZeroVector)
	, RightFootIKTarget(FVector::ZeroVector)
{
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
		if (!OwnerCharacter) return;
	}

	UpdateLocomotionData(DeltaSeconds);
	UpdateSurvivalData();
	UpdateCombatData();
	UpdateFootIK(DeltaSeconds);
	UpdateAdditiveWeights(DeltaSeconds);
}

void UPlayerAnimInstance::NativePostEvaluateAnimation()
{
	Super::NativePostEvaluateAnimation();
}

// ─────────────────────────────────────────────────────────────────────────────
// Locomotion
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
	UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
	if (!MovComp) return;

	const FVector Velocity = MovComp->Velocity;
	Speed = Velocity.Size2D();

	// Smooth speed for blend space transitions
	SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, SpeedSmoothingRate);

	// Direction angle relative to actor forward
	const FRotator ActorRot = OwnerCharacter->GetActorRotation();
	Direction = UKismetMathLibrary::NormalizedDeltaRotator(
		UKismetMathLibrary::MakeRotFromX(Velocity.GetSafeNormal()),
		ActorRot
	).Yaw;

	// Lean — based on velocity change (acceleration feel)
	const FVector VelocityDelta = Velocity - PreviousVelocity;
	const float RawLean = FVector::DotProduct(VelocityDelta, OwnerCharacter->GetActorRightVector()) * 0.05f;
	LeanAmount = FMath::FInterpTo(LeanAmount, FMath::Clamp(RawLean, -1.f, 1.f), DeltaSeconds, LeanSmoothingRate);
	PreviousVelocity = Velocity;

	VerticalVelocity = Velocity.Z;
	bIsMoving = Speed > WalkSpeedThreshold;
	bIsInAir = MovComp->IsFalling();
	bIsCrouching = MovComp->IsCrouching();

	// Landing flash
	if (bJustLanded)
	{
		LandingTimer -= DeltaSeconds;
		if (LandingTimer <= 0.f) bJustLanded = false;
	}

	LocomotionState = DetermineLocomotionState();
}

EAnim_LocomotionState UPlayerAnimInstance::DetermineLocomotionState() const
{
	if (bIsClimbing)  return EAnim_LocomotionState::Climb;
	if (bIsInAir)     return EAnim_LocomotionState::InAir;
	if (bJustLanded)  return EAnim_LocomotionState::Land;
	if (bIsCrouching)
	{
		return bIsMoving ? EAnim_LocomotionState::Sneak : EAnim_LocomotionState::Crouch;
	}
	if (bIsSneaking)  return EAnim_LocomotionState::Sneak;
	if (Speed >= SprintSpeedThreshold) return EAnim_LocomotionState::Sprint;
	if (Speed >= RunSpeedThreshold)    return EAnim_LocomotionState::Run;
	if (Speed >= WalkSpeedThreshold)   return EAnim_LocomotionState::Walk;
	return EAnim_LocomotionState::Idle;
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival / Condition
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateSurvivalData()
{
	// These values are driven by the TranspersonalCharacter survival stats.
	// If the character exposes them as BlueprintReadOnly floats we can cast and read;
	// for now we keep the last-set values (driven externally via Blueprint or C++ setter).
	bIsExhausted = StaminaNormalized < 0.15f;
	bIsInjured   = HealthNormalized  < 0.35f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Combat
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateCombatData()
{
	if (!OwnerCharacter) return;

	// Aim pitch/yaw from control rotation delta
	const FRotator ControlRot = OwnerCharacter->GetControlRotation();
	const FRotator ActorRot   = OwnerCharacter->GetActorRotation();
	const FRotator Delta      = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

	AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);
	AimYaw   = FMath::Clamp(Delta.Yaw,   -90.f, 90.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Foot IK — line-trace both feet to terrain surface
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	if (!bEnableFootIK || !OwnerCharacter) return;

	UWorld* World = OwnerCharacter->GetWorld();
	if (!World) return;

	// Only run IK when on ground
	if (bIsInAir)
	{
		PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.f, DeltaSeconds, FootIKInterpSpeed);
		return;
	}

	auto TraceFootIK = [&](const FName& BoneName, FVector& OutLocation, FRotator& OutRotation, FVector& IKTarget)
	{
		const FVector BoneLocation = OwnerCharacter->GetMesh()
			? OwnerCharacter->GetMesh()->GetSocketLocation(BoneName)
			: OwnerCharacter->GetActorLocation();

		const FVector TraceStart = BoneLocation + FVector(0.f, 0.f, FootIKTraceLength);
		const FVector TraceEnd   = BoneLocation - FVector(0.f, 0.f, FootIKTraceLength * 2.f);

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerCharacter);

		if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
		{
			const FVector TargetLoc = HitResult.ImpactPoint;
			IKTarget = FMath::VInterpTo(IKTarget, TargetLoc, DeltaSeconds, FootIKInterpSpeed);
			OutLocation = IKTarget;

			// Foot rotation from surface normal
			const FVector Normal = HitResult.ImpactNormal;
			OutRotation = FRotator(
				FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
				0.f,
				-FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z))
			);
		}
	};

	TraceFootIK(FName("foot_l"), LeftFootIKLocation,  LeftFootIKRotation,  LeftFootIKTarget);
	TraceFootIK(FName("foot_r"), RightFootIKLocation, RightFootIKRotation, RightFootIKTarget);

	// Pelvis offset = lowest foot delta so both feet can reach the ground
	const float LeftDelta  = LeftFootIKLocation.Z  - OwnerCharacter->GetActorLocation().Z;
	const float RightDelta = RightFootIKLocation.Z - OwnerCharacter->GetActorLocation().Z;
	const float TargetPelvis = FMath::Min(LeftDelta, RightDelta);
	PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, FootIKInterpSpeed * 0.5f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Additive weights — exhaustion, fear, injury blend layers
// ─────────────────────────────────────────────────────────────────────────────
void UPlayerAnimInstance::UpdateAdditiveWeights(float DeltaSeconds)
{
	const float ExhaustionTarget = bIsExhausted ? FMath::Clamp(1.f - StaminaNormalized, 0.f, 1.f) : 0.f;
	const float FearTarget       = FMath::Clamp(FearLevel, 0.f, 1.f);
	const float InjuryTarget     = bIsInjured   ? FMath::Clamp(1.f - HealthNormalized, 0.f, 1.f) : 0.f;

	AdditiveExhaustionWeight = FMath::FInterpTo(AdditiveExhaustionWeight, ExhaustionTarget, DeltaSeconds, 2.f);
	AdditiveFearWeight       = FMath::FInterpTo(AdditiveFearWeight,       FearTarget,       DeltaSeconds, 3.f);
	AdditiveInjuryWeight     = FMath::FInterpTo(AdditiveInjuryWeight,     InjuryTarget,     DeltaSeconds, 2.f);
}
