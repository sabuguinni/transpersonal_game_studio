// DinoSurvivorAnimInstance.cpp
// Animation Agent #10 — Transpersonal Game Studio
// Full AnimInstance implementation: locomotion state machine, foot IK, aim offset, survival blending

#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
	// Locomotion defaults
	LocomotionState = EAnim_LocomotionState::Idle;
	SurvivalPosture = EAnim_SurvivalPosture::Upright;
	Speed = 0.0f;
	Direction = 0.0f;
	LeanAngle = 0.0f;
	bIsInAir = false;
	bIsCrouching = false;
	bIsSprinting = false;
	bIsMoving = false;

	// Survival stats
	HealthNormalized = 1.0f;
	StaminaNormalized = 1.0f;
	FearNormalized = 0.0f;

	// Thresholds
	WalkSpeedThreshold = 10.0f;
	RunSpeedThreshold = 300.0f;
	SprintSpeedThreshold = 550.0f;

	// Foot IK
	bFootIKEnabled = true;
	FootIKTraceDistance = 80.0f;
	FootIKInterpSpeed = 15.0f;
	PelvisAdjustmentSpeed = 10.0f;

	// Aim offset
	AimPitch = 0.0f;
	AimYaw = 0.0f;
	AimOffsetInterpSpeed = 8.0f;

	// Upper body blend
	UpperBodyBlendWeight = 0.0f;
	SurvivalBlendWeight = 0.0f;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter || !MovementComponent)
	{
		OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
		if (OwnerCharacter)
			MovementComponent = OwnerCharacter->GetCharacterMovement();
		return;
	}

	UpdateLocomotionData(DeltaSeconds);
	UpdateLocomotionState();
	UpdateSurvivalPosture();
	UpdateAimOffset(DeltaSeconds);
	UpdateSurvivalBlending(DeltaSeconds);

	if (bFootIKEnabled)
	{
		UpdateFootIK(DeltaSeconds);
	}
}

// ─── LOCOMOTION ───────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
	FVector Velocity = OwnerCharacter->GetVelocity();
	Speed = Velocity.Size2D();
	bIsInAir = MovementComponent->IsFalling();
	bIsCrouching = OwnerCharacter->bIsCrouched;

	// Direction relative to actor facing
	if (Speed > WalkSpeedThreshold)
	{
		FRotator ActorRot = OwnerCharacter->GetActorRotation();
		FVector LocalVelocity = ActorRot.UnrotateVector(Velocity);
		Direction = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
		bIsMoving = true;
	}
	else
	{
		Direction = 0.0f;
		bIsMoving = false;
	}

	// Lean — lateral acceleration
	FVector Accel = MovementComponent->GetCurrentAcceleration();
	FVector LocalAccel = OwnerCharacter->GetActorRotation().UnrotateVector(Accel);
	float TargetLean = FMath::Clamp(LocalAccel.Y / MovementComponent->MaxAcceleration * 15.0f, -15.0f, 15.0f);
	LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 6.0f);
}

void UDinoSurvivorAnimInstance::UpdateLocomotionState()
{
	EAnim_LocomotionState NewState = LocomotionState;

	if (bIsInAir)
	{
		FVector Velocity = OwnerCharacter->GetVelocity();
		NewState = (Velocity.Z < -50.0f) ? EAnim_LocomotionState::Fall : EAnim_LocomotionState::Jump;
	}
	else if (bIsCrouching)
	{
		NewState = (Speed > WalkSpeedThreshold) ? EAnim_LocomotionState::CrouchWalk : EAnim_LocomotionState::Crouch;
	}
	else if (Speed >= SprintSpeedThreshold && bIsSprinting)
	{
		NewState = EAnim_LocomotionState::Sprint;
	}
	else if (Speed >= RunSpeedThreshold)
	{
		NewState = EAnim_LocomotionState::Run;
	}
	else if (Speed >= WalkSpeedThreshold)
	{
		NewState = EAnim_LocomotionState::Walk;
	}
	else
	{
		NewState = EAnim_LocomotionState::Idle;
	}

	// Landing transition
	if (LocomotionState == EAnim_LocomotionState::Fall && NewState == EAnim_LocomotionState::Idle)
	{
		NewState = EAnim_LocomotionState::Land;
	}

	LocomotionState = NewState;
}

void UDinoSurvivorAnimInstance::UpdateSurvivalPosture()
{
	// Determine posture from survival stats
	if (HealthNormalized < 0.2f)
	{
		SurvivalPosture = EAnim_SurvivalPosture::Wounded;
	}
	else if (StaminaNormalized < 0.15f)
	{
		SurvivalPosture = EAnim_SurvivalPosture::Exhausted;
	}
	else if (FearNormalized > 0.75f)
	{
		SurvivalPosture = EAnim_SurvivalPosture::Terrified;
	}
	else if (FearNormalized > 0.4f)
	{
		SurvivalPosture = EAnim_SurvivalPosture::Alert;
	}
	else
	{
		SurvivalPosture = EAnim_SurvivalPosture::Upright;
	}
}

// ─── FOOT IK ─────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	if (!OwnerCharacter) return;

	// Trace for left foot
	FAnim_FootIKData NewIKData;
	TraceFootIK(TEXT("foot_l"), NewIKData.LeftFootLocation, NewIKData.LeftFootRotation, NewIKData.bLeftFootGrounded);
	TraceFootIK(TEXT("foot_r"), NewIKData.RightFootLocation, NewIKData.RightFootRotation, NewIKData.bRightFootGrounded);

	// Interpolate foot locations
	FootIKData.LeftFootLocation = FMath::VInterpTo(FootIKData.LeftFootLocation, NewIKData.LeftFootLocation, DeltaSeconds, FootIKInterpSpeed);
	FootIKData.RightFootLocation = FMath::VInterpTo(FootIKData.RightFootLocation, NewIKData.RightFootLocation, DeltaSeconds, FootIKInterpSpeed);
	FootIKData.LeftFootRotation = FMath::RInterpTo(FootIKData.LeftFootRotation, NewIKData.LeftFootRotation, DeltaSeconds, FootIKInterpSpeed);
	FootIKData.RightFootRotation = FMath::RInterpTo(FootIKData.RightFootRotation, NewIKData.RightFootRotation, DeltaSeconds, FootIKInterpSpeed);
	FootIKData.bLeftFootGrounded = NewIKData.bLeftFootGrounded;
	FootIKData.bRightFootGrounded = NewIKData.bRightFootGrounded;

	// Pelvis adjustment — lower pelvis to accommodate lowest foot
	float LeftOffset = FootIKData.LeftFootLocation.Z - OwnerCharacter->GetActorLocation().Z;
	float RightOffset = FootIKData.RightFootLocation.Z - OwnerCharacter->GetActorLocation().Z;
	float TargetPelvisOffset = FMath::Min(LeftOffset, RightOffset);
	TargetPelvisOffset = FMath::Clamp(TargetPelvisOffset, -30.0f, 0.0f);

	FootIKData.PelvisOffset = FMath::FInterpTo(FootIKData.PelvisOffset, TargetPelvisOffset, DeltaSeconds, PelvisAdjustmentSpeed);
}

void UDinoSurvivorAnimInstance::TraceFootIK(const FName& BoneName, FVector& OutLocation, FRotator& OutRotation, bool& bGrounded)
{
	if (!OwnerCharacter) return;

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return;

	FVector BoneLocation = Mesh->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
	FVector TraceStart = FVector(BoneLocation.X, BoneLocation.Y, BoneLocation.Z + FootIKTraceDistance * 0.5f);
	FVector TraceEnd = FVector(BoneLocation.X, BoneLocation.Y, BoneLocation.Z - FootIKTraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	bGrounded = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams
	);

	if (bGrounded)
	{
		OutLocation = HitResult.ImpactPoint;
		// Compute foot rotation from surface normal
		FVector Normal = HitResult.ImpactNormal;
		FRotator SurfaceRotation = UKismetMathLibrary::MakeRotFromZX(Normal, OwnerCharacter->GetActorForwardVector());
		OutRotation = FRotator(SurfaceRotation.Pitch, 0.0f, SurfaceRotation.Roll);
	}
	else
	{
		OutLocation = BoneLocation;
		OutRotation = FRotator::ZeroRotator;
	}
}

// ─── AIM OFFSET ──────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
	if (!OwnerCharacter) return;

	AController* Controller = OwnerCharacter->GetController();
	if (!Controller) return;

	FRotator ControlRot = Controller->GetControlRotation();
	FRotator ActorRot = OwnerCharacter->GetActorRotation();
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

	float TargetPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
	float TargetYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);

	AimPitch = FMath::FInterpTo(AimPitch, TargetPitch, DeltaSeconds, AimOffsetInterpSpeed);
	AimYaw = FMath::FInterpTo(AimYaw, TargetYaw, DeltaSeconds, AimOffsetInterpSpeed);

	AimOffsetData.AimPitch = AimPitch;
	AimOffsetData.AimYaw = AimYaw;
}

// ─── SURVIVAL BLENDING ───────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalBlending(float DeltaSeconds)
{
	// Upper body weight: increases with fear, decreases with health
	float TargetUpperBody = FMath::Clamp(FearNormalized * 0.6f + (1.0f - HealthNormalized) * 0.4f, 0.0f, 1.0f);
	UpperBodyBlendWeight = FMath::FInterpTo(UpperBodyBlendWeight, TargetUpperBody, DeltaSeconds, 3.0f);

	// Survival blend: exhaustion/wound posture blend
	float TargetSurvival = 0.0f;
	switch (SurvivalPosture)
	{
		case EAnim_SurvivalPosture::Wounded:    TargetSurvival = 1.0f; break;
		case EAnim_SurvivalPosture::Exhausted:  TargetSurvival = 0.75f; break;
		case EAnim_SurvivalPosture::Terrified:  TargetSurvival = 0.5f; break;
		case EAnim_SurvivalPosture::Alert:      TargetSurvival = 0.25f; break;
		default:                                TargetSurvival = 0.0f; break;
	}
	SurvivalBlendWeight = FMath::FInterpTo(SurvivalBlendWeight, TargetSurvival, DeltaSeconds, 4.0f);
}

// ─── PUBLIC API ──────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::SetSurvivalStats(float Health, float Stamina, float Fear)
{
	HealthNormalized = FMath::Clamp(Health, 0.0f, 1.0f);
	StaminaNormalized = FMath::Clamp(Stamina, 0.0f, 1.0f);
	FearNormalized = FMath::Clamp(Fear, 0.0f, 1.0f);
}

void UDinoSurvivorAnimInstance::SetSprintingState(bool bSprinting)
{
	bIsSprinting = bSprinting;
}

EAnim_LocomotionState UDinoSurvivorAnimInstance::GetLocomotionState() const
{
	return LocomotionState;
}

FAnim_FootIKData UDinoSurvivorAnimInstance::GetFootIKData() const
{
	return FootIKData;
}
