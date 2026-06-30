#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UPlayerAnimInstance::UPlayerAnimInstance()
	: LocomotionState(EAnim_LocomotionState::Idle)
	, WeaponState(EAnim_WeaponState::Unarmed)
	, Speed(0.f)
	, SmoothedSpeed(0.f)
	, Direction(0.f)
	, LeanAmount(0.f)
	, VerticalVelocity(0.f)
	, bIsMoving(false)
	, bIsInAir(false)
	, bIsCrouching(false)
	, bIsSprinting(false)
	, bIsClimbing(false)
	, LeftFootIKLocation(FVector::ZeroVector)
	, RightFootIKLocation(FVector::ZeroVector)
	, LeftFootIKAlpha(0.f)
	, RightFootIKAlpha(0.f)
	, LeftFootIKRotation(FRotator::ZeroRotator)
	, RightFootIKRotation(FRotator::ZeroRotator)
	, AimPitch(0.f)
	, AimYaw(0.f)
	, ExhaustionAlpha(0.f)
	, InjuryAlpha(0.f)
	, bIsExhausted(false)
	, bIsInjured(false)
	, WalkSpeedThreshold(50.f)
	, RunSpeedThreshold(250.f)
	, SprintSpeedThreshold(500.f)
	, SpeedSmoothingRate(10.f)
	, IKTraceDistance(55.f)
	, LeanSmoothed(0.f)
	, PreviousSpeed(0.f)
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

	UpdateLocomotionState();
	UpdateIK(DeltaSeconds);
	UpdateAimOffset();
	UpdateSurvivalStates();
}

void UPlayerAnimInstance::UpdateLocomotionState()
{
	if (!OwnerCharacter) return;

	UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
	if (!MovComp) return;

	FVector Velocity = OwnerCharacter->GetVelocity();
	Speed = Velocity.Size2D();
	VerticalVelocity = Velocity.Z;

	// Smooth speed
	SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, GetWorld()->GetDeltaSeconds(), SpeedSmoothingRate);

	bIsMoving = Speed > WalkSpeedThreshold;
	bIsInAir = MovComp->IsFalling();
	bIsCrouching = MovComp->IsCrouching();

	// Direction (strafe angle)
	if (bIsMoving)
	{
		FRotator ActorRot = OwnerCharacter->GetActorRotation();
		FRotator VelocityRot = Velocity.Rotation();
		Direction = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRot, ActorRot).Yaw;
	}
	else
	{
		Direction = 0.f;
	}

	// Lean (rate of direction change)
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	if (DeltaTime > 0.f)
	{
		float RawLean = (Speed - PreviousSpeed) / DeltaTime * 0.01f;
		LeanSmoothed = FMath::FInterpTo(LeanSmoothed, RawLean, DeltaTime, 5.f);
		LeanAmount = FMath::Clamp(LeanSmoothed, -1.f, 1.f);
	}
	PreviousSpeed = Speed;

	// Locomotion state machine
	if (bIsInAir)
	{
		LocomotionState = EAnim_LocomotionState::InAir;
	}
	else if (bIsClimbing)
	{
		LocomotionState = EAnim_LocomotionState::Climb;
	}
	else if (bIsCrouching)
	{
		LocomotionState = bIsMoving ? EAnim_LocomotionState::Sneak : EAnim_LocomotionState::Crouch;
	}
	else if (Speed >= SprintSpeedThreshold)
	{
		LocomotionState = EAnim_LocomotionState::Sprint;
	}
	else if (Speed >= RunSpeedThreshold)
	{
		LocomotionState = EAnim_LocomotionState::Run;
	}
	else if (Speed >= WalkSpeedThreshold)
	{
		LocomotionState = EAnim_LocomotionState::Walk;
	}
	else
	{
		LocomotionState = EAnim_LocomotionState::Idle;
	}
}

void UPlayerAnimInstance::UpdateIK(float DeltaSeconds)
{
	if (!OwnerCharacter || bIsInAir) 
	{
		LeftFootIKAlpha = FMath::FInterpTo(LeftFootIKAlpha, 0.f, DeltaSeconds, 10.f);
		RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.f, DeltaSeconds, 10.f);
		return;
	}

	PerformFootIKTrace(FName("foot_l"), LeftFootIKLocation, LeftFootIKRotation, LeftFootIKAlpha);
	PerformFootIKTrace(FName("foot_r"), RightFootIKLocation, RightFootIKRotation, RightFootIKAlpha);
}

void UPlayerAnimInstance::PerformFootIKTrace(FName SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha)
{
	if (!OwnerCharacter) return;

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return;

	FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
	FVector TraceStart = SocketLocation + FVector(0.f, 0.f, IKTraceDistance);
	FVector TraceEnd   = SocketLocation - FVector(0.f, 0.f, IKTraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd,
		ECollisionChannel::ECC_Visibility, Params
	);

	if (bHit)
	{
		OutLocation = HitResult.ImpactPoint;
		FVector Normal = HitResult.ImpactNormal;
		OutRotation = FRotator(
			FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)),
			0.f,
			-FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z))
		);
		OutAlpha = FMath::FInterpTo(OutAlpha, 1.f, GetWorld()->GetDeltaSeconds(), 10.f);
	}
	else
	{
		OutAlpha = FMath::FInterpTo(OutAlpha, 0.f, GetWorld()->GetDeltaSeconds(), 10.f);
	}
}

void UPlayerAnimInstance::UpdateAimOffset()
{
	if (!OwnerCharacter) return;

	AController* Controller = OwnerCharacter->GetController();
	if (!Controller) return;

	FRotator ControlRot = Controller->GetControlRotation();
	FRotator ActorRot   = OwnerCharacter->GetActorRotation();
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

	AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);
	AimYaw   = FMath::Clamp(Delta.Yaw,   -90.f, 90.f);
}

void UPlayerAnimInstance::UpdateSurvivalStates()
{
	// Survival stats integration — reads from character if available
	// Defaults to 0 until TranspersonalCharacter exposes stamina/health
	// These alphas drive additive animation layers for exhaustion/injury posture
	bIsExhausted = ExhaustionAlpha > 0.7f;
	bIsInjured   = InjuryAlpha   > 0.5f;
}
