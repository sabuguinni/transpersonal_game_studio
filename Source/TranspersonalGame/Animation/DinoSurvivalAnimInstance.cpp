#include "DinoSurvivalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivalAnimInstance::UDinoSurvivalAnimInstance()
{
}

void UDinoSurvivalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACharacter>(GetOwningActor());
	if (OwnerCharacter)
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UDinoSurvivalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter || !MovementComponent)
	{
		OwnerCharacter = Cast<ACharacter>(GetOwningActor());
		if (OwnerCharacter)
		{
			MovementComponent = OwnerCharacter->GetCharacterMovement();
		}
		return;
	}

	UpdateLocomotion(DeltaSeconds);
	UpdateFootIK(DeltaSeconds);
	UpdateAimOffset();
	UpdateSurvivalState();
}

void UDinoSurvivalAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
	if (!MovementComponent) return;

	// Ground speed from velocity (ignore Z)
	FVector Velocity = MovementComponent->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();

	// Moving threshold
	bIsMoving = GroundSpeed > 10.0f;

	// In air
	bIsInAir = MovementComponent->IsFalling();

	// Crouching
	bIsCrouching = MovementComponent->IsCrouching();

	// Sprinting: speed > 450 cm/s (roughly 16 km/h — prehistoric sprint)
	bIsSprinting = GroundSpeed > 450.0f;

	// Movement direction relative to character facing
	if (bIsMoving)
	{
		FRotator CharRotation = OwnerCharacter->GetActorRotation();
		FRotator VelocityRotation = Velocity.Rotation();
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, CharRotation);
		MovementDirection = DeltaRotation.Yaw;
	}
	else
	{
		MovementDirection = 0.0f;
	}
}

void UDinoSurvivalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	if (!OwnerCharacter) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// Only apply foot IK when on ground
	if (bIsInAir)
	{
		LeftFootIKLocation = FVector::ZeroVector;
		RightFootIKLocation = FVector::ZeroVector;
		PelvisOffset = 0.0f;
		return;
	}

	// Get foot bone world locations via mesh component
	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return;

	// Trace from left foot
	FVector LeftFootWorld = Mesh->GetBoneLocation(FName("foot_l"));
	FVector RightFootWorld = Mesh->GetBoneLocation(FName("foot_r"));

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(OwnerCharacter);

	// Left foot trace
	FHitResult LeftHit;
	FVector LeftStart = LeftFootWorld + FVector(0, 0, IKTraceDistance);
	FVector LeftEnd = LeftFootWorld - FVector(0, 0, IKTraceDistance);

	if (World->LineTraceSingleByChannel(LeftHit, LeftStart, LeftEnd, ECC_Visibility, TraceParams))
	{
		FVector TargetLeft = LeftHit.ImpactPoint;
		LeftFootIKCached = FMath::VInterpTo(LeftFootIKCached, TargetLeft, DeltaSeconds, IKInterpSpeed);
		LeftFootIKLocation = LeftFootIKCached;

		// Foot rotation from surface normal
		FVector Normal = LeftHit.ImpactNormal;
		LeftFootIKRotation = FRotator(
			FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
			0.0f,
			-FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z))
		);
	}

	// Right foot trace
	FHitResult RightHit;
	FVector RightStart = RightFootWorld + FVector(0, 0, IKTraceDistance);
	FVector RightEnd = RightFootWorld - FVector(0, 0, IKTraceDistance);

	if (World->LineTraceSingleByChannel(RightHit, RightStart, RightEnd, ECC_Visibility, TraceParams))
	{
		FVector TargetRight = RightHit.ImpactPoint;
		RightFootIKCached = FMath::VInterpTo(RightFootIKCached, TargetRight, DeltaSeconds, IKInterpSpeed);
		RightFootIKLocation = RightFootIKCached;

		FVector Normal = RightHit.ImpactNormal;
		RightFootIKRotation = FRotator(
			FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
			0.0f,
			-FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z))
		);
	}

	// Pelvis offset: lower pelvis to the lowest foot
	float LeftDelta = LeftFootIKLocation.Z - LeftFootWorld.Z;
	float RightDelta = RightFootIKLocation.Z - RightFootWorld.Z;
	PelvisOffset = FMath::Min(LeftDelta, RightDelta);
}

void UDinoSurvivalAnimInstance::UpdateAimOffset()
{
	if (!OwnerCharacter) return;

	AController* Controller = OwnerCharacter->GetController();
	if (!Controller) return;

	FRotator ControlRotation = Controller->GetControlRotation();
	FRotator ActorRotation = OwnerCharacter->GetActorRotation();
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

	AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
	AimYaw = FMath::ClampAngle(DeltaRot.Yaw, -180.0f, 180.0f);
}

void UDinoSurvivalAnimInstance::UpdateSurvivalState()
{
	// Survival stats are read from the owning character if it exposes them
	// For now, derive fear from proximity to fast-moving threats
	// Full integration with TranspersonalCharacter survival stats in next cycle

	// Stamina affects animation speed multiplier (handled in blend space)
	// Fear affects upper body tension (handled in aim offset blend)
}
