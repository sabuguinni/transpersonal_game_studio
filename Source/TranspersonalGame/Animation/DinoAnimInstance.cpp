#include "DinoAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UDinoAnimInstance::UDinoAnimInstance()
{
	WalkSpeedThreshold = 150.0f;
	TrotSpeedThreshold = 400.0f;
	RunSpeedThreshold = 700.0f;
	FootIKInterpSpeed = 12.0f;
}

void UDinoAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerPawn = TryGetPawnOwner();
}

void UDinoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	OwnerPawn = TryGetPawnOwner();
	if (!OwnerPawn)
	{
		return;
	}

	// --- Locomotion ---
	FVector Velocity = OwnerPawn->GetVelocity();
	GroundSpeed = Velocity.Size2D();
	bIsMoving = GroundSpeed > 10.0f;

	// Movement direction relative to actor forward
	if (bIsMoving)
	{
		FVector ForwardVector = OwnerPawn->GetActorForwardVector();
		FVector VelocityNorm = Velocity.GetSafeNormal2D();
		MovementDirection = FMath::RadiansToDegrees(
			FMath::Atan2(
				FVector::CrossProduct(ForwardVector, VelocityNorm).Z,
				FVector::DotProduct(ForwardVector, VelocityNorm)
			)
		);
	}
	else
	{
		MovementDirection = 0.0f;
	}

	UpdateLocomotionGait();
	UpdateFootIK(DeltaSeconds);
}

void UDinoAnimInstance::UpdateLocomotionGait()
{
	if (!bIsMoving)
	{
		if (!bIsAttacking && !bIsRoaring)
		{
			CurrentGait = EAnim_DinoGait::Idle;
		}
		return;
	}

	if (GroundSpeed < WalkSpeedThreshold)
	{
		CurrentGait = EAnim_DinoGait::Walk;
	}
	else if (GroundSpeed < TrotSpeedThreshold)
	{
		CurrentGait = EAnim_DinoGait::Trot;
	}
	else
	{
		CurrentGait = EAnim_DinoGait::Run;
	}
}

void UDinoAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	if (!OwnerPawn)
	{
		return;
	}

	UWorld* World = OwnerPawn->GetWorld();
	if (!World)
	{
		return;
	}

	// Foot IK via line traces downward from approximate foot bone positions
	FVector ActorLocation = OwnerPawn->GetActorLocation();
	FVector ActorForward = OwnerPawn->GetActorForwardVector();
	FVector ActorRight = OwnerPawn->GetActorRightVector();

	// Approximate foot offsets (scaled to actor size)
	float FootOffset = 80.0f;
	float TraceLength = 200.0f;

	FVector LeftFootStart = ActorLocation + ActorRight * (-FootOffset);
	FVector RightFootStart = ActorLocation + ActorRight * FootOffset;

	FHitResult LeftHit, RightHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);

	bool bLeftHit = World->LineTraceSingleByChannel(
		LeftHit,
		LeftFootStart + FVector(0, 0, 50.0f),
		LeftFootStart - FVector(0, 0, TraceLength),
		ECC_WorldStatic,
		Params
	);

	bool bRightHit = World->LineTraceSingleByChannel(
		RightHit,
		RightFootStart + FVector(0, 0, 50.0f),
		RightFootStart - FVector(0, 0, TraceLength),
		ECC_WorldStatic,
		Params
	);

	// Interpolate foot IK targets
	FVector TargetLeft = bLeftHit ? LeftHit.ImpactPoint : LeftFootStart;
	FVector TargetRight = bRightHit ? RightHit.ImpactPoint : RightFootStart;

	LeftFootIKLocation = FMath::VInterpTo(LeftFootIKLocation, TargetLeft, DeltaSeconds, FootIKInterpSpeed);
	RightFootIKLocation = FMath::VInterpTo(RightFootIKLocation, TargetRight, DeltaSeconds, FootIKInterpSpeed);

	// Body lean based on foot height difference
	float HeightDiff = LeftFootIKLocation.Z - RightFootIKLocation.Z;
	BodyLeanAngle = FMath::Clamp(HeightDiff * 0.1f, -15.0f, 15.0f);
}

void UDinoAnimInstance::TriggerAttack()
{
	bIsAttacking = true;
	CurrentGait = EAnim_DinoGait::Attack;
}

void UDinoAnimInstance::TriggerRoar()
{
	bIsRoaring = true;
	CurrentGait = EAnim_DinoGait::Roar;
}

void UDinoAnimInstance::ClearCombatState()
{
	bIsAttacking = false;
	bIsRoaring = false;
	// Gait will be re-evaluated next tick
}

FAnim_DinoLocomotionState UDinoAnimInstance::GetLocomotionState() const
{
	FAnim_DinoLocomotionState State;
	State.Speed = GroundSpeed;
	State.bIsMoving = bIsMoving;
	State.bIsAttacking = bIsAttacking;
	State.bIsAlerted = bIsAlerted;
	State.TurnRate = MovementDirection;
	return State;
}
