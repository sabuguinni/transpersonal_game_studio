#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
	Speed = 0.f;
	Direction = 0.f;
	LeanAngle = 0.f;
	LeanAngleSmoothed = 0.f;
	SpeedSmoothed = 0.f;
	bIsInAir = false;
	bIsCrouching = false;
	bIsSprinting = false;
	bIsMoving = false;

	AimPitch = 0.f;
	AimYaw = 0.f;

	LeftFootIKLocation = FVector::ZeroVector;
	RightFootIKLocation = FVector::ZeroVector;
	LeftFootIKRotation = FRotator::ZeroRotator;
	RightFootIKRotation = FRotator::ZeroRotator;
	LeftFootIKAlpha = 0.f;
	RightFootIKAlpha = 0.f;
	PelvisOffset = FVector::ZeroVector;

	StaminaNormalized = 1.f;
	FearNormalized = 0.f;
	bIsExhausted = false;
	bIsInjured = false;
	bIsDead = false;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (DeltaSeconds <= 0.f)
	{
		return;
	}

	UpdateLocomotion(DeltaSeconds);
	UpdateFootIK(DeltaSeconds);
	UpdateAimOffset(DeltaSeconds);
	UpdateSurvivalState(DeltaSeconds);
}

void UDinoSurvivorAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
	ACharacter* Character = Cast<ACharacter>(TryGetPawnOwner());
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovComp = Character->GetCharacterMovement();
	if (!MovComp)
	{
		return;
	}

	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	float RawSpeed = Velocity.Size();

	// Smooth speed
	SpeedSmoothed = FMath::FInterpTo(SpeedSmoothed, RawSpeed, DeltaSeconds, 8.f);
	Speed = SpeedSmoothed;

	bIsMoving = Speed > 10.f;
	bIsInAir = MovComp->IsFalling();
	bIsCrouching = MovComp->IsCrouching();

	// Sprint: speed > 400 (walk ~200, run ~400, sprint ~600)
	bIsSprinting = Speed > 420.f;

	// Direction relative to character facing
	if (bIsMoving)
	{
		FRotator ActorRot = Character->GetActorRotation();
		FRotator VelocityRot = Velocity.Rotation();
		Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
	}
	else
	{
		Direction = FMath::FInterpTo(Direction, 0.f, DeltaSeconds, 5.f);
	}

	// Lean angle based on lateral acceleration
	FVector Accel = MovComp->GetCurrentAcceleration();
	float LateralAccel = FVector::DotProduct(Accel, Character->GetActorRightVector());
	float TargetLean = FMath::Clamp(LateralAccel * 0.02f, -15.f, 15.f);
	LeanAngleSmoothed = FMath::FInterpTo(LeanAngleSmoothed, TargetLean, DeltaSeconds, 4.f);
	LeanAngle = LeanAngleSmoothed;
}

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	ACharacter* Character = Cast<ACharacter>(TryGetPawnOwner());
	if (!Character || !Character->GetMesh())
	{
		return;
	}

	UWorld* World = Character->GetWorld();
	if (!World)
	{
		return;
	}

	// Only apply foot IK when on ground
	if (bIsInAir)
	{
		LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.f, DeltaSeconds, 10.f);
		RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.f, DeltaSeconds, 10.f);
		PelvisOffset = FMath::VInterpTo(PelvisOffset, FVector::ZeroVector, DeltaSeconds, 10.f);
		return;
	}

	const FName LeftFootBone = TEXT("foot_l");
	const FName RightFootBone = TEXT("foot_r");
	const float TraceHalfHeight = 50.f;
	const float FootHeightOffset = 10.f;

	auto TraceFootIK = [&](FName BoneName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
	{
		FVector BoneLocation = Character->GetMesh()->GetBoneLocation(BoneName);
		FVector TraceStart = BoneLocation + FVector(0.f, 0.f, TraceHalfHeight);
		FVector TraceEnd = BoneLocation - FVector(0.f, 0.f, TraceHalfHeight);

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Character);

		bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

		if (bHit)
		{
			FVector TargetLoc = HitResult.ImpactPoint + FVector(0.f, 0.f, FootHeightOffset);
			OutLocation = FMath::VInterpTo(OutLocation, TargetLoc, DeltaSeconds, 15.f);

			// Foot rotation from surface normal
			FVector Normal = HitResult.ImpactNormal;
			FRotator TargetRot = FRotator(
				FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
				0.f,
				-FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z))
			);
			OutRotation = FMath::RInterpTo(OutRotation, TargetRot, DeltaSeconds, 15.f);
			OutAlpha = FMath::FInterpTo(OutAlpha, 1.f, DeltaSeconds, 10.f);
		}
		else
		{
			OutAlpha = FMath::FInterpTo(OutAlpha, 0.f, DeltaSeconds, 10.f);
		}
	};

	TraceFootIK(LeftFootBone, LeftFootIKLocation, LeftFootIKRotation, LeftFootIKAlpha);
	TraceFootIK(RightFootBone, RightFootIKLocation, RightFootIKRotation, RightFootIKAlpha);

	// Pelvis offset: lower pelvis to accommodate foot on lower surface
	float LeftDelta = LeftFootIKLocation.Z - Character->GetActorLocation().Z;
	float RightDelta = RightFootIKLocation.Z - Character->GetActorLocation().Z;
	float MinDelta = FMath::Min(LeftDelta, RightDelta);
	FVector TargetPelvis = FVector(0.f, 0.f, FMath::Clamp(MinDelta, -20.f, 0.f));
	PelvisOffset = FMath::VInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, 10.f);
}

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		return;
	}

	AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		return;
	}

	FRotator ControlRot = Controller->GetControlRotation();
	FRotator ActorRot = Pawn->GetActorRotation();
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

	AimPitch = FMath::FInterpTo(AimPitch, FMath::Clamp(Delta.Pitch, -90.f, 90.f), DeltaSeconds, 15.f);
	AimYaw = FMath::FInterpTo(AimYaw, FMath::Clamp(Delta.Yaw, -90.f, 90.f), DeltaSeconds, 15.f);
}

void UDinoSurvivorAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
	// Read survival stats from character if available
	// Defaults maintained if character doesn't expose these
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		return;
	}

	// Try to read Health/Stamina/Fear via reflection — graceful fallback if not present
	// These are set externally by TranspersonalCharacter when it updates its stats
	// bIsExhausted: stamina < 0.1
	bIsExhausted = StaminaNormalized < 0.1f;

	// bIsInjured: implied by external systems setting StaminaNormalized low
	// bIsDead: set externally
}
