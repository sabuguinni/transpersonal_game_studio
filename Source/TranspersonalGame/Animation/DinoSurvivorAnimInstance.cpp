#include "DinoSurvivorAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
	// Defaults set in header via member initializers
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
		if (!OwnerCharacter) return;
	}

	UpdateLocomotion(DeltaSeconds);
	UpdateFootIK();
	UpdateAimOffset();
	UpdateSurvivalState();
}

// ─────────────────────────────────────────────────────────────────────────────
// Locomotion
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
	UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
	if (!MovComp) return;

	// Ground speed (ignore Z)
	FVector Velocity = MovComp->Velocity;
	Speed = FVector(Velocity.X, Velocity.Y, 0.f).Size();

	// Direction relative to actor forward
	FRotator ActorRot = OwnerCharacter->GetActorRotation();
	Direction = UKismetMathLibrary::NormalizedDeltaRotator(
		UKismetMathLibrary::MakeRotFromX(Velocity),
		ActorRot
	).Yaw;

	// Lean — rate of direction change
	float RawLean = (Speed - PreviousSpeed) * 0.01f;
	SmoothedLean = FMath::FInterpTo(SmoothedLean, RawLean, DeltaSeconds, 6.f);
	LeanAngle = FMath::Clamp(SmoothedLean, -1.f, 1.f);
	PreviousSpeed = Speed;

	// State flags
	bIsInAir = MovComp->IsFalling();
	bIsCrouching = MovComp->IsCrouching();
	bIsSprinting = (Speed > SprintThreshold);
}

// ─────────────────────────────────────────────────────────────────────────────
// Foot IK
// ─────────────────────────────────────────────────────────────────────────────

FVector UDinoSurvivorAnimInstance::TraceFootIK(FName SocketName, bool& bHit) const
{
	bHit = false;
	if (!OwnerCharacter) return FVector::ZeroVector;

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return FVector::ZeroVector;

	FVector SocketLoc = Mesh->GetSocketLocation(SocketName);
	FVector TraceStart = SocketLoc + FVector(0.f, 0.f, FootIKTraceDistance);
	FVector TraceEnd   = SocketLoc - FVector(0.f, 0.f, FootIKTraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd,
		ECC_Visibility, Params
	);

	return bHit ? HitResult.ImpactPoint : SocketLoc;
}

void UDinoSurvivorAnimInstance::UpdateFootIK()
{
	// Disable IK when airborne — feet should be free
	if (bIsInAir)
	{
		LeftFootIKAlpha  = FMath::FInterpTo(LeftFootIKAlpha,  0.f, GetWorld()->GetDeltaSeconds(), 8.f);
		RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, 0.f, GetWorld()->GetDeltaSeconds(), 8.f);
		PelvisOffset = 0.f;
		return;
	}

	bool bLeftHit  = false;
	bool bRightHit = false;

	FVector LeftTarget  = TraceFootIK(FName("foot_l"), bLeftHit);
	FVector RightTarget = TraceFootIK(FName("foot_r"), bRightHit);

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Smooth IK targets
	LeftFootIKLocation  = FMath::VInterpTo(LeftFootIKLocation,  LeftTarget,  DeltaTime, 12.f);
	RightFootIKLocation = FMath::VInterpTo(RightFootIKLocation, RightTarget, DeltaTime, 12.f);

	// Alpha: full IK when grounded and moving slowly (prevents sliding)
	float TargetAlpha = (Speed < SprintThreshold) ? 1.f : 0.f;
	LeftFootIKAlpha  = FMath::FInterpTo(LeftFootIKAlpha,  bLeftHit  ? TargetAlpha : 0.f, DeltaTime, 8.f);
	RightFootIKAlpha = FMath::FInterpTo(RightFootIKAlpha, bRightHit ? TargetAlpha : 0.f, DeltaTime, 8.f);

	// Pelvis offset — lower pelvis to accommodate the lower foot
	float LeftDelta  = LeftFootIKLocation.Z  - OwnerCharacter->GetActorLocation().Z;
	float RightDelta = RightFootIKLocation.Z - OwnerCharacter->GetActorLocation().Z;
	float TargetPelvis = FMath::Min(LeftDelta, RightDelta);
	PelvisOffset = FMath::FInterpTo(PelvisOffset, FMath::Clamp(TargetPelvis, -30.f, 0.f), DeltaTime, 10.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Aim Offset
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateAimOffset()
{
	if (!OwnerCharacter) return;

	AController* Controller = OwnerCharacter->GetController();
	if (!Controller) return;

	FRotator ControlRot  = Controller->GetControlRotation();
	FRotator ActorRot    = OwnerCharacter->GetActorRotation();
	FRotator DeltaRot    = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	AimPitch = FMath::FInterpTo(AimPitch, FMath::Clamp(DeltaRot.Pitch, -90.f, 90.f),   DeltaTime, 15.f);
	AimYaw   = FMath::FInterpTo(AimYaw,   FMath::Clamp(DeltaRot.Yaw,   -90.f, 90.f),   DeltaTime, 15.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival State
// ─────────────────────────────────────────────────────────────────────────────

void UDinoSurvivorAnimInstance::UpdateSurvivalState()
{
	// Read survival stats from the owning character via reflection
	// This avoids a hard dependency on TranspersonalCharacter header
	if (!OwnerCharacter) return;

	// Try to read Stamina property via UE reflection
	UClass* CharClass = OwnerCharacter->GetClass();

	// Stamina
	FFloatProperty* StaminaProp = FindFProperty<FFloatProperty>(CharClass, TEXT("Stamina"));
	if (StaminaProp)
	{
		float RawStamina = StaminaProp->GetPropertyValue_InContainer(OwnerCharacter);
		// Assume max stamina 100 — normalise
		StaminaNormalized = FMath::Clamp(RawStamina / 100.f, 0.f, 1.f);
	}
	else
	{
		StaminaNormalized = 1.f;
	}

	bIsExhausted = (StaminaNormalized < 0.2f);

	// Fear
	FFloatProperty* FearProp = FindFProperty<FFloatProperty>(CharClass, TEXT("Fear"));
	if (FearProp)
	{
		float RawFear = FearProp->GetPropertyValue_InContainer(OwnerCharacter);
		FearNormalized = FMath::Clamp(RawFear / 100.f, 0.f, 1.f);
	}
	else
	{
		FearNormalized = 0.f;
	}

	bIsTerrified = (FearNormalized > 0.7f);

	// Health / injured
	FFloatProperty* HealthProp = FindFProperty<FFloatProperty>(CharClass, TEXT("Health"));
	if (HealthProp)
	{
		float RawHealth = HealthProp->GetPropertyValue_InContainer(OwnerCharacter);
		bIsInjured = (RawHealth < 30.f);
	}
	else
	{
		bIsInjured = false;
	}
}
