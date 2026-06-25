#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
	GroundSpeed = 0.f;
	LateralSpeed = 0.f;
	bIsMoving = false;
	bIsSprinting = false;
	bIsCrouching = false;
	bIsInAir = false;
	bIsFalling = false;
	StaminaNormalized = 1.f;
	FearLevel = 0.f;
	bIsExhausted = false;
	MovementDirection = 0.f;
	LeftFootIKLocation = FVector::ZeroVector;
	RightFootIKLocation = FVector::ZeroVector;
	LeftFootIKAlpha = 0.f;
	RightFootIKAlpha = 0.f;
	WalkRunBlend = 0.f;
	SprintBlend = 0.f;
	OwnerCharacter = nullptr;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(GetOwningActor());
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(GetOwningActor());
		if (!OwnerCharacter) return;
	}

	UpdateLocomotionState(DeltaSeconds);
	UpdateFootIK(DeltaSeconds);
	UpdateSurvivalState(DeltaSeconds);
}

void UTranspersonalAnimInstance::UpdateLocomotionState(float DeltaSeconds)
{
	UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
	if (!MovComp) return;

	const FVector Velocity = MovComp->Velocity;
	GroundSpeed = Velocity.Size2D();
	LateralSpeed = FMath::Abs(Velocity.Y);
	bIsMoving = GroundSpeed > 3.f;
	bIsInAir = MovComp->IsMovingOnGround() == false;
	bIsFalling = MovComp->IsFalling();
	bIsCrouching = MovComp->IsCrouching();

	// Sprint detection — speed above walk threshold (300 cm/s walk, 600 run, 900 sprint)
	bIsSprinting = GroundSpeed > 650.f && !bIsInAir && !bIsExhausted;

	// Blend weights for blend space
	// WalkRunBlend: 0=idle, 0.5=walk, 1.0=run
	const float WalkSpeed = 300.f;
	const float RunSpeed  = 600.f;
	if (GroundSpeed < WalkSpeed)
	{
		WalkRunBlend = FMath::GetMappedRangeValueClamped(FVector2D(0.f, WalkSpeed), FVector2D(0.f, 0.5f), GroundSpeed);
	}
	else
	{
		WalkRunBlend = FMath::GetMappedRangeValueClamped(FVector2D(WalkSpeed, RunSpeed), FVector2D(0.5f, 1.f), GroundSpeed);
	}

	// SprintBlend: 0=run, 1=sprint
	SprintBlend = FMath::GetMappedRangeValueClamped(FVector2D(RunSpeed, 900.f), FVector2D(0.f, 1.f), GroundSpeed);

	// Movement direction (for strafe blend space)
	MovementDirection = CalculateMovementDirection();
}

float UTranspersonalAnimInstance::CalculateMovementDirection() const
{
	if (!OwnerCharacter) return 0.f;

	const FVector Velocity = OwnerCharacter->GetVelocity();
	if (Velocity.SizeSquared2D() < 1.f) return 0.f;

	const FRotator ActorRot = OwnerCharacter->GetActorRotation();
	const FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);

	// Angle in degrees: 0=forward, +90=right, -90=left
	return FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	// Trace both feet to terrain and compute IK offset
	TraceFootIK(FName("foot_l"), LeftFootIKLocation, LeftFootIKAlpha);
	TraceFootIK(FName("foot_r"), RightFootIKLocation, RightFootIKAlpha);
}

bool UTranspersonalAnimInstance::TraceFootIK(const FName& SocketName, FVector& OutLocation, float& OutAlpha) const
{
	if (!OwnerCharacter) return false;

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return false;

	const FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
	const FVector TraceStart = SocketLocation + FVector(0.f, 0.f, 50.f);
	const FVector TraceEnd   = SocketLocation - FVector(0.f, 0.f, 100.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	UWorld* World = OwnerCharacter->GetWorld();
	if (!World) return false;

	const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

	if (bHit)
	{
		OutLocation = HitResult.ImpactPoint;
		// Alpha: 1 when on uneven terrain, 0 when flat
		const float HeightDiff = FMath::Abs(HitResult.ImpactPoint.Z - SocketLocation.Z);
		OutAlpha = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 30.f), FVector2D(0.f, 1.f), HeightDiff);
		return true;
	}

	OutLocation = SocketLocation;
	OutAlpha = 0.f;
	return false;
}

void UTranspersonalAnimInstance::UpdateSurvivalState(float DeltaSeconds)
{
	// Default values — real survival stats hooked up via TranspersonalCharacter
	// when that class exposes stamina/fear properties
	// For now: derive exhaustion from speed vs stamina heuristic
	bIsExhausted = (StaminaNormalized < 0.15f);

	// Fear modifies animation urgency — handled in blend space via FearLevel
	// High fear = more erratic, faster transitions
}
