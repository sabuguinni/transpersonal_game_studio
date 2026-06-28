#include "DinosaurAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UDinosaurAnimInstance::UDinosaurAnimInstance()
	: GroundSpeed(0.f)
	, MovementDirection(0.f)
	, bIsInAir(false)
	, bIsChasing(false)
	, bIsPatrolling(true)
	, bIsAttacking(false)
	, bIsDying(false)
	, HealthNormalized(1.f)
	, bIsRoaring(false)
	, LeftFootIKTarget(FVector::ZeroVector)
	, RightFootIKTarget(FVector::ZeroVector)
	, PelvisOffset(0.f)
	, SpeciesTag(NAME_None)
	, WalkThreshold(150.f)
	, RunThreshold(500.f)
	, OwnerPawn(nullptr)
	, MovementComp(nullptr)
	, SmoothedSpeed(0.f)
	, RoarCooldown(0.f)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeInitializeAnimation
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerPawn = TryGetPawnOwner();
	if (!OwnerPawn)
	{
		return;
	}

	// Try to get movement component — works for both ACharacter and APawn subclasses
	MovementComp = Cast<UCharacterMovementComponent>(
		OwnerPawn->GetMovementComponent());
}

// ─────────────────────────────────────────────────────────────────────────────
// NativeUpdateAnimation — called every frame
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerPawn)
	{
		OwnerPawn = TryGetPawnOwner();
		if (!OwnerPawn) return;
	}

	if (!MovementComp)
	{
		MovementComp = Cast<UCharacterMovementComponent>(
			OwnerPawn->GetMovementComponent());
	}

	// Tick roar cooldown
	if (RoarCooldown > 0.f)
	{
		RoarCooldown -= DeltaSeconds;
	}

	UpdateLocomotion(DeltaSeconds);
	UpdateFootIK();
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateLocomotion
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
	if (!OwnerPawn) return;

	const FVector Velocity = OwnerPawn->GetVelocity();
	const float RawSpeed = Velocity.Size2D();

	// Smooth speed to avoid animation jitter on rapid velocity changes
	const float SmoothRate = 8.f;
	SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, RawSpeed, DeltaSeconds, SmoothRate);
	GroundSpeed = SmoothedSpeed;

	// Air state
	if (MovementComp)
	{
		bIsInAir = MovementComp->IsFalling();
	}

	// Movement direction (degrees, -180..180)
	MovementDirection = ComputeMovementDirection();

	// Locomotion mode flags
	if (!bIsDying && !bIsAttacking)
	{
		bIsChasing = (GroundSpeed >= RunThreshold);
		bIsPatrolling = (GroundSpeed > WalkThreshold && GroundSpeed < RunThreshold);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateFootIK — line trace per foot socket
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::UpdateFootIK()
{
	if (!OwnerPawn || bIsInAir || bIsDying) return;

	USkeletalMeshComponent* Mesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh) return;

	UWorld* World = OwnerPawn->GetWorld();
	if (!World) return;

	const float TraceUp   = 60.f;
	const float TraceDown = 80.f;
	const float InterpSpeed = 12.f;
	const float DeltaTime = World->GetDeltaSeconds();

	// Helper lambda — traces from a foot socket and returns the hit Z offset
	auto TraceFootSocket = [&](const FName& SocketName, FVector& OutTarget) -> bool
	{
		if (!Mesh->DoesSocketExist(SocketName)) return false;

		const FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
		const FVector Start = SocketLoc + FVector(0.f, 0.f, TraceUp);
		const FVector End   = SocketLoc - FVector(0.f, 0.f, TraceDown);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerPawn);

		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			const float ZOffset = Hit.ImpactPoint.Z - SocketLoc.Z;
			OutTarget = FMath::VInterpTo(OutTarget,
				FVector(0.f, 0.f, ZOffset), DeltaTime, InterpSpeed);
			return true;
		}
		return false;
	};

	FVector NewLeft  = LeftFootIKTarget;
	FVector NewRight = RightFootIKTarget;

	bool bLeftHit  = TraceFootSocket(FName("foot_l"), NewLeft);
	bool bRightHit = TraceFootSocket(FName("foot_r"), NewRight);

	LeftFootIKTarget  = NewLeft;
	RightFootIKTarget = NewRight;

	// Pelvis offset — lower pelvis to the lowest foot so neither foot floats
	if (bLeftHit || bRightHit)
	{
		const float LowestFoot = FMath::Min(NewLeft.Z, NewRight.Z);
		PelvisOffset = FMath::FInterpTo(PelvisOffset, LowestFoot, DeltaTime, InterpSpeed);
	}
	else
	{
		PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.f, DeltaTime, InterpSpeed);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeMovementDirection
// ─────────────────────────────────────────────────────────────────────────────

float UDinosaurAnimInstance::ComputeMovementDirection() const
{
	if (!OwnerPawn) return 0.f;

	const FVector Velocity = OwnerPawn->GetVelocity();
	if (Velocity.SizeSquared2D() < 1.f) return 0.f;

	const FRotator ActorRot = OwnerPawn->GetActorRotation();
	const FVector ForwardDir = ActorRot.Vector();
	const FVector RightDir   = FRotationMatrix(ActorRot).GetScaledAxis(EAxis::Y);

	const FVector VelNorm = Velocity.GetSafeNormal2D();
	const float   ForwardDot = FVector::DotProduct(ForwardDir, VelNorm);
	const float   RightDot   = FVector::DotProduct(RightDir,   VelNorm);

	return FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
}

// ─────────────────────────────────────────────────────────────────────────────
// Blueprint-callable combat triggers
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurAnimInstance::TriggerRoar()
{
	if (bIsDying) return;
	if (RoarCooldown > 0.f) return;

	bIsRoaring = true;
	RoarCooldown = 8.f; // seconds between roars

	// The Animation Blueprint state machine will reset bIsRoaring after the montage ends
	// via a Notify — we set a timer here as a safety fallback
	if (UWorld* World = GetWorld())
	{
		FTimerHandle RoarTimer;
		World->GetTimerManager().SetTimer(RoarTimer, [this]()
		{
			bIsRoaring = false;
		}, 3.5f, false);
	}
}

void UDinosaurAnimInstance::TriggerAttack()
{
	if (bIsDying) return;

	bIsAttacking = true;

	// Safety reset after attack window
	if (UWorld* World = GetWorld())
	{
		FTimerHandle AttackTimer;
		World->GetTimerManager().SetTimer(AttackTimer, [this]()
		{
			bIsAttacking = false;
		}, 1.2f, false);
	}
}

void UDinosaurAnimInstance::TriggerDeath()
{
	bIsDying     = true;
	bIsAttacking = false;
	bIsRoaring   = false;
	bIsChasing   = false;
	bIsPatrolling = false;
	GroundSpeed  = 0.f;
	SmoothedSpeed = 0.f;
}
