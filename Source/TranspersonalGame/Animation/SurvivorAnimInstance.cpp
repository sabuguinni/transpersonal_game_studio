#include "SurvivorAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

USurvivorAnimInstance::USurvivorAnimInstance()
{
	// Locomotion defaults
	Speed              = 0.0f;
	Direction          = 0.0f;
	bIsMoving          = false;
	bIsSprinting       = false;
	bIsCrouching       = false;

	// Airborne defaults
	bIsInAir           = false;
	VerticalVelocity   = 0.0f;

	// Survival defaults
	StaminaRatio       = 1.0f;
	FearLevel          = 0.0f;
	bIsInjured         = false;

	// Foot IK defaults
	LeftFootIKLocation  = FVector::ZeroVector;
	RightFootIKLocation = FVector::ZeroVector;
	FootIKAlpha         = 0.0f;
	IKInterpSpeed       = 12.0f;

	// Thresholds
	IdleThreshold       = 10.0f;
	SprintThreshold     = 400.0f;
	FootIKTraceDistance = 80.0f;

	// Cached pointers
	OwnerCharacter    = nullptr;
	MovementComponent = nullptr;
}

void USurvivorAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Pawn = TryGetPawnOwner();
	if (Pawn)
	{
		OwnerCharacter    = Cast<ACharacter>(Pawn);
		MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
	}
}

void USurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter || !MovementComponent)
	{
		// Re-attempt cache in case the character spawned after init
		APawn* Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			OwnerCharacter    = Cast<ACharacter>(Pawn);
			MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
		}
		return;
	}

	UpdateLocomotion();
	UpdateSurvivalState();
	UpdateFootIK(DeltaSeconds);
}

// ─────────────────────────────────────────────────────────────────────────────
// Locomotion
// ─────────────────────────────────────────────────────────────────────────────

void USurvivorAnimInstance::UpdateLocomotion()
{
	const FVector Velocity    = MovementComponent->Velocity;
	const FVector HorizVel    = FVector(Velocity.X, Velocity.Y, 0.0f);
	Speed                     = HorizVel.Size();
	VerticalVelocity          = Velocity.Z;

	bIsMoving   = Speed > IdleThreshold;
	bIsSprinting = Speed > SprintThreshold;
	bIsInAir    = MovementComponent->IsFalling();
	bIsCrouching = MovementComponent->IsCrouching();

	// Calculate strafe direction relative to character facing
	if (bIsMoving)
	{
		const FRotator ActorRot = OwnerCharacter->GetActorRotation();
		const FVector  LocalVel = ActorRot.UnrotateVector(HorizVel.GetSafeNormal());
		Direction = FMath::Clamp(LocalVel.Y, -1.0f, 1.0f);
	}
	else
	{
		Direction = 0.0f;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival State
// ─────────────────────────────────────────────────────────────────────────────

void USurvivorAnimInstance::UpdateSurvivalState()
{
	// Try to read survival stats from the character via a generic property approach.
	// If TranspersonalCharacter exposes Stamina/Health/Fear as floats, we read them here.
	// Fallback: defaults remain (StaminaRatio=1, FearLevel=0, bIsInjured=false).

	if (!OwnerCharacter)
	{
		return;
	}

	// Use UObject reflection to safely read properties without hard coupling
	// to TranspersonalCharacter header (avoids circular dependency).
	static const FName PropStamina(TEXT("Stamina"));
	static const FName PropMaxStamina(TEXT("MaxStamina"));
	static const FName PropHealth(TEXT("Health"));
	static const FName PropMaxHealth(TEXT("MaxHealth"));
	static const FName PropFear(TEXT("FearLevel"));

	auto ReadFloat = [&](const FName& PropName, float Default) -> float
	{
		FFloatProperty* Prop = FindFieldChecked<FFloatProperty>(OwnerCharacter->GetClass(), PropName);
		if (Prop)
		{
			return Prop->GetPropertyValue_InContainer(OwnerCharacter);
		}
		return Default;
	};

	// Stamina ratio
	const float Stamina    = ReadFloat(PropStamina,    100.0f);
	const float MaxStamina = ReadFloat(PropMaxStamina, 100.0f);
	StaminaRatio = (MaxStamina > 0.0f) ? FMath::Clamp(Stamina / MaxStamina, 0.0f, 1.0f) : 1.0f;

	// Injury check
	const float Health    = ReadFloat(PropHealth,    100.0f);
	const float MaxHealth = ReadFloat(PropMaxHealth, 100.0f);
	const float HealthRatio = (MaxHealth > 0.0f) ? FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f) : 1.0f;
	bIsInjured = HealthRatio < 0.2f;

	// Fear level
	FearLevel = FMath::Clamp(ReadFloat(PropFear, 0.0f), 0.0f, 1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Foot IK
// ─────────────────────────────────────────────────────────────────────────────

void USurvivorAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	if (!OwnerCharacter)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh)
	{
		return;
	}

	UWorld* World = OwnerCharacter->GetWorld();
	if (!World)
	{
		return;
	}

	// Only apply IK when grounded
	const float TargetIKAlpha = bIsInAir ? 0.0f : 1.0f;
	FootIKAlpha = FMath::FInterpTo(FootIKAlpha, TargetIKAlpha, DeltaSeconds, IKInterpSpeed);

	if (FootIKAlpha < 0.01f)
	{
		return;
	}

	// Bone names for the survivor skeleton (Mannequin-compatible)
	static const FName LeftFootBone(TEXT("foot_l"));
	static const FName RightFootBone(TEXT("foot_r"));

	auto TraceFootIK = [&](const FName& BoneName) -> FVector
	{
		const FVector BoneWorldLoc = Mesh->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
		const FVector TraceStart   = BoneWorldLoc + FVector(0.0f, 0.0f, FootIKTraceDistance * 0.5f);
		const FVector TraceEnd     = BoneWorldLoc - FVector(0.0f, 0.0f, FootIKTraceDistance);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerCharacter);

		const bool bHit = World->LineTraceSingleByChannel(
			Hit, TraceStart, TraceEnd,
			ECC_Visibility, Params
		);

		if (bHit)
		{
			// Return hit location in component space
			return Mesh->GetComponentTransform().InverseTransformPosition(Hit.Location);
		}

		return Mesh->GetComponentTransform().InverseTransformPosition(BoneWorldLoc);
	};

	LeftFootIKLocation  = TraceFootIK(LeftFootBone);
	RightFootIKLocation = TraceFootIK(RightFootBone);
}
