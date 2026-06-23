#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
	RunThreshold = 300.0f;
	SprintThreshold = 500.0f;
	FootIKTraceDistance = 50.0f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cache owner references — safe to call here, world exists at this point
	APawn* OwnerPawn = TryGetPawnOwner();
	if (OwnerPawn)
	{
		OwnerCharacter = Cast<ACharacter>(OwnerPawn);
		if (OwnerCharacter)
		{
			MovementComponent = OwnerCharacter->GetCharacterMovement();
		}
	}
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter || !MovementComponent)
	{
		// Re-attempt cache in case of late initialization
		APawn* OwnerPawn = TryGetPawnOwner();
		if (OwnerPawn)
		{
			OwnerCharacter = Cast<ACharacter>(OwnerPawn);
			if (OwnerCharacter)
			{
				MovementComponent = OwnerCharacter->GetCharacterMovement();
			}
		}
		return;
	}

	// ── Velocity & Speed ────────────────────────────────────────────────────
	const FVector Velocity = MovementComponent->Velocity;
	Speed = Velocity.Size2D(); // Horizontal speed only

	// ── Direction (for strafe blending) ─────────────────────────────────────
	if (Speed > 1.0f)
	{
		const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
		Direction = UKismetMathLibrary::NormalizedDeltaRotator(
			Velocity.Rotation(),
			ActorRotation
		).Yaw;
	}
	else
	{
		Direction = 0.0f;
	}

	// ── Air State ────────────────────────────────────────────────────────────
	bIsInAir = MovementComponent->IsFalling();

	// ── Acceleration ─────────────────────────────────────────────────────────
	bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;

	// ── Crouch ───────────────────────────────────────────────────────────────
	bIsCrouching = OwnerCharacter->bIsCrouched;

	// ── Sprint detection ─────────────────────────────────────────────────────
	bIsSprinting = Speed >= SprintThreshold;

	// ── Foot IK (only when grounded) ─────────────────────────────────────────
	if (!bIsInAir)
	{
		UpdateFootIK(DeltaSeconds);
	}
	else
	{
		// Smoothly disable IK while airborne
		LeftFootIKAlpha  = InterpFootIKAlpha(LeftFootIKAlpha,  0.0f, DeltaSeconds);
		RightFootIKAlpha = InterpFootIKAlpha(RightFootIKAlpha, 0.0f, DeltaSeconds);
	}
}

void UTranspersonalAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	if (!OwnerCharacter)
	{
		return;
	}

	// Left foot
	FVector LeftHit;
	bool bLeftHit = TraceFootIK(FName("foot_l"), LeftHit);
	if (bLeftHit)
	{
		LeftFootIKLocation = LeftHit;
		LeftFootIKAlpha = InterpFootIKAlpha(LeftFootIKAlpha, 1.0f, DeltaSeconds);
	}
	else
	{
		LeftFootIKAlpha = InterpFootIKAlpha(LeftFootIKAlpha, 0.0f, DeltaSeconds);
	}

	// Right foot
	FVector RightHit;
	bool bRightHit = TraceFootIK(FName("foot_r"), RightHit);
	if (bRightHit)
	{
		RightFootIKLocation = RightHit;
		RightFootIKAlpha = InterpFootIKAlpha(RightFootIKAlpha, 1.0f, DeltaSeconds);
	}
	else
	{
		RightFootIKAlpha = InterpFootIKAlpha(RightFootIKAlpha, 0.0f, DeltaSeconds);
	}
}

bool UTranspersonalAnimInstance::TraceFootIK(FName SocketName, FVector& OutLocation)
{
	if (!OwnerCharacter)
	{
		return false;
	}

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh)
	{
		return false;
	}

	const FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
	const FVector TraceStart = SocketLocation + FVector(0.0f, 0.0f, FootIKTraceDistance);
	const FVector TraceEnd   = SocketLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		OutLocation = HitResult.ImpactPoint;
		return true;
	}

	return false;
}

float UTranspersonalAnimInstance::InterpFootIKAlpha(float Current, float Target, float DeltaSeconds, float Speed)
{
	return FMath::FInterpTo(Current, Target, DeltaSeconds, Speed);
}
