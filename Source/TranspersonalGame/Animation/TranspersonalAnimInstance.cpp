#include "TranspersonalAnimInstance.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
	: GroundSpeed(0.f)
	, LateralDirection(0.f)
	, bIsMoving(false)
	, bIsInAir(false)
	, bIsSprinting(false)
	, bIsCrouching(false)
	, HealthNormalized(1.f)
	, StaminaNormalized(1.f)
	, FearLevel(0.f)
	, bIsInjured(false)
	, bIsPanicking(false)
	, bIsArmed(false)
	, bIsAttacking(false)
	, FootIKAlpha(1.f)
	, LeftFootIKTarget(FVector::ZeroVector)
	, RightFootIKTarget(FVector::ZeroVector)
	, PelvisOffset(0.f)
	, AimPitch(0.f)
	, AimYaw(0.f)
	, OwnerCharacter(nullptr)
{
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ATranspersonalCharacter>(TryGetPawnOwner());
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ATranspersonalCharacter>(TryGetPawnOwner());
		if (!OwnerCharacter) return;
	}

	UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
	if (!MovComp) return;

	// ── Locomotion ──────────────────────────────────────────────────────────

	const FVector Velocity = OwnerCharacter->GetVelocity();
	const FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.f);
	GroundSpeed = HorizontalVelocity.Size();
	bIsMoving = GroundSpeed > 3.f;
	bIsInAir = MovComp->IsFalling();
	bIsCrouching = OwnerCharacter->bIsCrouched;

	// Lateral direction: dot product of velocity with character right vector
	if (bIsMoving)
	{
		const FVector RightVec = OwnerCharacter->GetActorRightVector();
		const FVector VelNorm = HorizontalVelocity.GetSafeNormal();
		LateralDirection = FVector::DotProduct(VelNorm, RightVec);
	}
	else
	{
		LateralDirection = SmoothFloat(LateralDirection, 0.f, 8.f, DeltaSeconds);
	}

	// Sprint threshold: walk speed ~300, run ~600
	bIsSprinting = GroundSpeed > 450.f && !bIsInAir;

	// ── Survival States ─────────────────────────────────────────────────────

	// Read survival stats from character (properties exposed via UPROPERTY)
	// Use safe accessors — character may not have initialised stats yet
	if (OwnerCharacter->GetClass()->FindPropertyByName(FName("Health")))
	{
		const float* HealthPtr = OwnerCharacter->GetClass()
			->FindPropertyByName(FName("Health"))
			->ContainerPtrToValuePtr<float>(OwnerCharacter);
		const float* MaxHealthPtr = OwnerCharacter->GetClass()
			->FindPropertyByName(FName("MaxHealth"))
			->ContainerPtrToValuePtr<float>(OwnerCharacter);
		if (HealthPtr && MaxHealthPtr && *MaxHealthPtr > 0.f)
		{
			HealthNormalized = FMath::Clamp(*HealthPtr / *MaxHealthPtr, 0.f, 1.f);
		}
	}

	if (OwnerCharacter->GetClass()->FindPropertyByName(FName("Stamina")))
	{
		const float* StamPtr = OwnerCharacter->GetClass()
			->FindPropertyByName(FName("Stamina"))
			->ContainerPtrToValuePtr<float>(OwnerCharacter);
		const float* MaxStamPtr = OwnerCharacter->GetClass()
			->FindPropertyByName(FName("MaxStamina"))
			->ContainerPtrToValuePtr<float>(OwnerCharacter);
		if (StamPtr && MaxStamPtr && *MaxStamPtr > 0.f)
		{
			StaminaNormalized = FMath::Clamp(*StamPtr / *MaxStamPtr, 0.f, 1.f);
		}
	}

	if (OwnerCharacter->GetClass()->FindPropertyByName(FName("Fear")))
	{
		const float* FearPtr = OwnerCharacter->GetClass()
			->FindPropertyByName(FName("Fear"))
			->ContainerPtrToValuePtr<float>(OwnerCharacter);
		if (FearPtr)
		{
			FearLevel = FMath::Clamp(*FearPtr / 100.f, 0.f, 1.f);
		}
	}

	bIsInjured = HealthNormalized < 0.3f;
	bIsPanicking = FearLevel > 0.7f;

	// ── Foot IK ─────────────────────────────────────────────────────────────

	// Only do foot IK when grounded and not sprinting (performance guard)
	if (!bIsInAir && !bIsSprinting)
	{
		float LeftPelvisDelta = 0.f;
		float RightPelvisDelta = 0.f;

		LeftFootIKTarget = TraceFootIK(FName("foot_l"), LeftPelvisDelta);
		RightFootIKTarget = TraceFootIK(FName("foot_r"), RightPelvisDelta);

		// Pelvis drops to the lower foot
		const float TargetPelvisOffset = FMath::Min(LeftPelvisDelta, RightPelvisDelta);
		PelvisOffset = SmoothFloat(PelvisOffset, TargetPelvisOffset, 10.f, DeltaSeconds);

		FootIKAlpha = SmoothFloat(FootIKAlpha, 1.f, 5.f, DeltaSeconds);
	}
	else
	{
		FootIKAlpha = SmoothFloat(FootIKAlpha, 0.f, 5.f, DeltaSeconds);
		PelvisOffset = SmoothFloat(PelvisOffset, 0.f, 10.f, DeltaSeconds);
	}

	// ── Aim Offset ──────────────────────────────────────────────────────────

	const FRotator ControlRot = OwnerCharacter->GetControlRotation();
	const FRotator ActorRot = OwnerCharacter->GetActorRotation();
	const FRotator DeltaRot = (ControlRot - ActorRot).GetNormalized();

	AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.f, 90.f);
	AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -90.f, 90.f);
}

FVector UTranspersonalAnimInstance::TraceFootIK(const FName& FootSocketName, float& OutPelvisDelta) const
{
	OutPelvisDelta = 0.f;

	USkeletalMeshComponent* MeshComp = GetSkelMeshComponent();
	if (!MeshComp || !OwnerCharacter) return FVector::ZeroVector;

	const FVector SocketLocation = MeshComp->GetSocketLocation(FootSocketName);

	// Trace downward from above the foot
	const FVector TraceStart = SocketLocation + FVector(0.f, 0.f, 50.f);
	const FVector TraceEnd = SocketLocation - FVector(0.f, 0.f, 75.f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bTraceComplex = false;

	const bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		// How far the foot needs to move down from its animated position
		OutPelvisDelta = HitResult.Location.Z - SocketLocation.Z;
		return HitResult.Location;
	}

	return SocketLocation;
}

float UTranspersonalAnimInstance::SmoothFloat(float Current, float Target, float Speed, float DeltaTime)
{
	return FMath::FInterpTo(Current, Target, DeltaTime, Speed);
}
