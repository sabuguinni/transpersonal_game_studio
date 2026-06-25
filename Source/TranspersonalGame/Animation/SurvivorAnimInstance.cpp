#include "SurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

USurvivorAnimInstance::USurvivorAnimInstance()
{
	Speed              = 0.0f;
	Direction          = 0.0f;
	bIsInAir           = false;
	bIsCrouching       = false;
	bIsSprinting       = false;
	bIsMoving          = false;
	FearLevel          = 0.0f;
	StaminaNormalized  = 1.0f;
	HealthNormalized   = 1.0f;
	bIsArmed           = false;
	AimPitch           = 0.0f;
	AimYaw             = 0.0f;
	PelvisOffset       = 0.0f;
	bEnableFootIK      = true;
	FootIKInterpSpeed  = 15.0f;
	OwnerCharacter     = nullptr;
	MovementComponent  = nullptr;
}

void USurvivorAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	OwnerCharacter = Cast<ACharacter>(Pawn);
	if (OwnerCharacter)
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void USurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter || !MovementComponent) return;

	// --- LOCOMOTION ---
	FVector Velocity = MovementComponent->Velocity;
	Speed = Velocity.Size2D();
	bIsMoving  = Speed > 10.0f;
	bIsInAir   = MovementComponent->IsFalling();
	bIsCrouching = OwnerCharacter->bIsCrouched;

	// Sprint threshold — 300+ cm/s
	bIsSprinting = Speed > 300.0f;

	// Direction: angle between character forward and velocity
	if (bIsMoving)
	{
		FRotator ActorRot = OwnerCharacter->GetActorRotation();
		FVector ForwardVector = ActorRot.Vector();
		FVector VelocityNorm = Velocity.GetSafeNormal2D();
		float DotProduct = FVector::DotProduct(ForwardVector, VelocityNorm);
		float CrossZ = ForwardVector.X * VelocityNorm.Y - ForwardVector.Y * VelocityNorm.X;
		Direction = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
		if (CrossZ < 0.0f) Direction = -Direction;
	}
	else
	{
		Direction = 0.0f;
	}

	// --- AIM OFFSET ---
	// Derive pitch/yaw from control rotation vs actor rotation
	AController* Controller = OwnerCharacter->GetController();
	if (Controller)
	{
		FRotator ControlRot = Controller->GetControlRotation();
		FRotator ActorRot   = OwnerCharacter->GetActorRotation();
		FRotator Delta       = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
		AimPitch = FMath::Clamp(Delta.Pitch, -90.0f, 90.0f);
		AimYaw   = FMath::Clamp(Delta.Yaw,   -90.0f, 90.0f);
	}

	// --- FOOT IK ---
	if (bEnableFootIK && !bIsInAir)
	{
		UpdateFootIK(DeltaSeconds);
	}
	else
	{
		// Reset IK when in air
		LeftFootIKOffset  = FMath::VInterpTo(LeftFootIKOffset,  FVector::ZeroVector, DeltaSeconds, FootIKInterpSpeed);
		RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, FVector::ZeroVector, DeltaSeconds, FootIKInterpSpeed);
		PelvisOffset      = FMath::FInterpTo(PelvisOffset,      0.0f,                DeltaSeconds, FootIKInterpSpeed);
	}
}

void USurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	FVector LeftTarget  = FVector::ZeroVector;
	FVector RightTarget = FVector::ZeroVector;

	bool bLeftHit  = TraceFootPosition(FName("foot_l"), LeftTarget);
	bool bRightHit = TraceFootPosition(FName("foot_r"), RightTarget);

	// Smooth interpolation toward targets
	if (bLeftHit)
	{
		LeftFootIKOffsetTarget = LeftTarget;
	}
	if (bRightHit)
	{
		RightFootIKOffsetTarget = RightTarget;
	}

	LeftFootIKOffset  = FMath::VInterpTo(LeftFootIKOffset,  LeftFootIKOffsetTarget,  DeltaSeconds, FootIKInterpSpeed);
	RightFootIKOffset = FMath::VInterpTo(RightFootIKOffset, RightFootIKOffsetTarget, DeltaSeconds, FootIKInterpSpeed);

	// Pelvis offset = lowest foot to avoid leg stretching
	float LowestFoot = FMath::Min(LeftFootIKOffset.Z, RightFootIKOffset.Z);
	PelvisOffset = FMath::FInterpTo(PelvisOffset, LowestFoot, DeltaSeconds, FootIKInterpSpeed);
}

bool USurvivorAnimInstance::TraceFootPosition(FName SocketName, FVector& OutIKOffset)
{
	if (!OwnerCharacter) return false;

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return false;

	FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
	FVector TraceStart     = SocketLocation + FVector(0.0f, 0.0f, 50.0f);
	FVector TraceEnd       = SocketLocation - FVector(0.0f, 0.0f, 75.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	UWorld* World = OwnerCharacter->GetWorld();
	if (!World) return false;

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECollisionChannel::ECC_Visibility,
		Params
	);

	if (bHit)
	{
		// Offset in world Z — how much to adjust the foot
		float ZDiff = HitResult.Location.Z - SocketLocation.Z;
		OutIKOffset = FVector(0.0f, 0.0f, ZDiff);
		return true;
	}

	return false;
}
