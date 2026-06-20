#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
	Speed = 0.f;
	Direction = 0.f;
	LeanAngle = 0.f;
	bIsInAir = false;
	bIsCrouching = false;
	bIsSprinting = false;
	bIsMoving = false;

	LeftFootIKLocation = FVector::ZeroVector;
	RightFootIKLocation = FVector::ZeroVector;
	LeftFootIKRotation = FRotator::ZeroRotator;
	RightFootIKRotation = FRotator::ZeroRotator;
	PelvisOffset = 0.f;
	LeftFootIKAlpha = 0.f;
	RightFootIKAlpha = 0.f;

	AimPitch = 0.f;
	AimYaw = 0.f;

	StaminaNormalized = 1.f;
	FearNormalized = 0.f;
	bIsExhausted = false;
	bIsInjured = false;
	bIsFleeing = false;

	SmoothedSpeed = 0.f;
	SmoothedDirection = 0.f;

	LeanAngleVelocity = 0.f;
	SpeedInterpVelocity = 0.f;
	DirectionInterpVelocity = 0.f;

	OwnerCharacter = nullptr;
	MovementComponent = nullptr;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
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

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter || !MovementComponent)
	{
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

	UpdateLocomotion(DeltaSeconds);
	UpdateFootIK(DeltaSeconds);
	UpdateAimOffset(DeltaSeconds);
	UpdateSurvivalState(DeltaSeconds);
}

void UDinoSurvivorAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
	FVector Velocity = MovementComponent->Velocity;
	FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.f);

	// Raw speed
	Speed = HorizontalVelocity.Size();

	// Smooth speed with interp
	SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, 8.f);

	// Direction relative to character facing
	FRotator CharRotation = OwnerCharacter->GetActorRotation();
	FVector LocalVelocity = CharRotation.UnrotateVector(HorizontalVelocity);
	Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));

	// Smooth direction
	SmoothedDirection = FMath::FInterpTo(SmoothedDirection, Direction, DeltaSeconds, 10.f);

	// Lean angle — based on angular velocity
	float AngularVelocityZ = MovementComponent->GetLastUpdateRotation().Yaw;
	float TargetLean = FMath::Clamp(AngularVelocityZ * 0.05f, -15.f, 15.f);
	LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 5.f);

	// State flags
	bIsInAir = MovementComponent->IsFalling();
	bIsCrouching = MovementComponent->IsCrouching();
	bIsMoving = Speed > 10.f;

	// Sprint detection — speed above walk threshold
	const float SprintThreshold = 375.f;
	bIsSprinting = Speed > SprintThreshold && !bIsInAir;
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	if (bIsInAir)
	{
		// No IK in air — blend out
		LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.f, DeltaSeconds, 10.f);
		RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.f, DeltaSeconds, 10.f);
		PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.f, DeltaSeconds, 10.f);
		return;
	}

	FVector LeftNormal, RightNormal;
	FVector LeftHit = TraceFootIK(FName("foot_l"), LeftNormal);
	FVector RightHit = TraceFootIK(FName("foot_r"), RightNormal);

	// Convert world hit to component-local offset
	FVector ActorLocation = OwnerCharacter->GetActorLocation();
	float LeftOffset = LeftHit.Z - ActorLocation.Z;
	float RightOffset = RightHit.Z - ActorLocation.Z;

	// Pelvis drops to accommodate the lower foot
	float LowestFoot = FMath::Min(LeftOffset, RightOffset);
	float TargetPelvis = FMath::Clamp(LowestFoot, -20.f, 0.f);
	PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, 12.f);

	// IK target locations (relative to pelvis-adjusted position)
	LeftFootIKLocation = FMath::VInterpTo(LeftFootIKLocation, LeftHit, DeltaSeconds, 15.f);
	RightFootIKLocation = FMath::VInterpTo(RightFootIKLocation, RightHit, DeltaSeconds, 15.f);

	// IK rotations from surface normal
	FRotator LeftRot = FRotationMatrix::MakeFromZX(LeftNormal, OwnerCharacter->GetActorForwardVector()).Rotator();
	FRotator RightRot = FRotationMatrix::MakeFromZX(RightNormal, OwnerCharacter->GetActorForwardVector()).Rotator();
	LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, LeftRot, DeltaSeconds, 15.f);
	RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, RightRot, DeltaSeconds, 15.f);

	// Blend in IK when on ground and moving slowly
	float TargetAlpha = bIsMoving ? 0.7f : 1.0f;
	LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, TargetAlpha, DeltaSeconds, 8.f);
	RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, TargetAlpha, DeltaSeconds, 8.f);
}

FVector UDinoSurvivorAnimInstance::TraceFootIK(FName SocketName, FVector& OutHitNormal)
{
	OutHitNormal = FVector::UpVector;

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh)
	{
		return OwnerCharacter->GetActorLocation();
	}

	FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
	FVector TraceStart = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z + 50.f);
	FVector TraceEnd = FVector(SocketLocation.X, SocketLocation.Y, SocketLocation.Z - 75.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	if (bHit)
	{
		OutHitNormal = HitResult.Normal;
		return HitResult.ImpactPoint;
	}

	return SocketLocation;
}

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
	AController* Controller = OwnerCharacter->GetController();
	if (!Controller)
	{
		return;
	}

	FRotator ControlRotation = Controller->GetControlRotation();
	FRotator ActorRotation = OwnerCharacter->GetActorRotation();
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

	AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.f, 90.f);
	AimYaw = FMath::Clamp(DeltaRot.Yaw, -90.f, 90.f);
}

void UDinoSurvivorAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
	// Default values — will be driven by TranspersonalCharacter survival stats
	// when that system is fully wired. For now, derive from movement state.

	// Exhaustion: low stamina approximated by prolonged sprinting
	if (bIsSprinting)
	{
		StaminaNormalized = FMath::FInterpTo(StaminaNormalized, 0.f, DeltaSeconds, 0.05f);
	}
	else
	{
		StaminaNormalized = FMath::FInterpTo(StaminaNormalized, 1.f, DeltaSeconds, 0.1f);
	}

	bIsExhausted = StaminaNormalized < 0.15f;

	// Injured state affects movement — check for reduced max walk speed
	const float InjuredSpeedThreshold = 250.f;
	bIsInjured = MovementComponent->MaxWalkSpeed < InjuredSpeedThreshold;

	// Fear — elevated when sprinting at low stamina
	float TargetFear = (bIsSprinting && StaminaNormalized < 0.3f) ? 1.f : 0.f;
	FearNormalized = FMath::FInterpTo(FearNormalized, TargetFear, DeltaSeconds, 2.f);

	bIsFleeing = FearNormalized > 0.5f && bIsSprinting;
}
