#include "TranspersonalAnimBlueprint.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// ─────────────────────────────────────────────────────────────────────────────
UTranspersonalAnimBlueprint::UTranspersonalAnimBlueprint()
{
	// Default constructor — nothing to initialise here; UE5 calls
	// NativeInitializeAnimation() once the owning pawn is set.
}

// ─────────────────────────────────────────────────────────────────────────────
void UTranspersonalAnimBlueprint::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) { return; }

	OwnerCharacter    = Cast<ACharacter>(Pawn);
	MovementComponent = OwnerCharacter
	                  ? OwnerCharacter->GetCharacterMovement()
	                  : nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
void UTranspersonalAnimBlueprint::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Re-cache on first valid tick (handles hot-reload / PIE re-entry)
	if (!OwnerCharacter || !MovementComponent)
	{
		NativeInitializeAnimation();
		return;
	}

	// ── Velocity & speed ─────────────────────────────────────────────────
	const FVector Velocity = MovementComponent->Velocity;
	GroundSpeed            = Velocity.Size2D();          // horizontal speed
	VerticalVelocity       = Velocity.Z;

	// ── Movement direction (relative to actor forward) ───────────────────
	if (GroundSpeed > 1.f)
	{
		const FRotator ActorRot  = OwnerCharacter->GetActorRotation();
		const FVector  VelNorm   = Velocity.GetSafeNormal2D();
		const FVector  Forward   = ActorRot.Vector();
		const FVector  Right     = FRotationMatrix(ActorRot).GetScaledAxis(EAxis::Y);

		const float DotForward = FVector::DotProduct(VelNorm, Forward);
		const float DotRight   = FVector::DotProduct(VelNorm, Right);
		MovementDirection = FMath::RadiansToDegrees(FMath::Atan2(DotRight, DotForward));
	}
	else
	{
		MovementDirection = 0.f;
	}

	// ── Air state ────────────────────────────────────────────────────────
	const bool bCurrentlyInAir = MovementComponent->IsFalling();

	// Detect landing (was in air last frame, now on ground)
	bJustLanded = bWasInAir && !bCurrentlyInAir;
	bWasInAir   = bCurrentlyInAir;
	bIsInAir    = bCurrentlyInAir;

	if (bIsInAir)
	{
		TimeInAir += DeltaSeconds;
	}
	else
	{
		TimeInAir = 0.f;
	}

	// ── Locomotion flags ─────────────────────────────────────────────────
	bIsRunning   = GroundSpeed > WalkSpeedThreshold;
	bIsSprinting = GroundSpeed > SprintSpeedThreshold;
	bIsCrouching = MovementComponent->IsCrouching();

	// ── Survival state — read from character if it exposes these floats ──
	// We use a soft interface: try to find properties via reflection.
	// If the character doesn't expose them, we default to 0.
	// This keeps the AnimInstance decoupled from a specific character class.
	static const FName PropHealth   = TEXT("Health");
	static const FName PropStamina  = TEXT("Stamina");
	static const FName PropFear     = TEXT("Fear");

	// Health → InjuryAlpha  (low health = more injury animation)
	if (float* HealthPtr = reinterpret_cast<float*>(
		OwnerCharacter->GetClass()->FindPropertyByName(PropHealth)
		? OwnerCharacter->GetClass()->FindPropertyByName(PropHealth)
		  ->ContainerPtrToValuePtr<float>(OwnerCharacter)
		: nullptr))
	{
		const float NormHealth = FMath::Clamp(*HealthPtr / 100.f, 0.f, 1.f);
		InjuryAlpha = FMath::InterpEaseIn(0.f, 1.f, 1.f - NormHealth, 2.f);
	}

	// Stamina → FatigueAlpha
	if (float* StaminaPtr = reinterpret_cast<float*>(
		OwnerCharacter->GetClass()->FindPropertyByName(PropStamina)
		? OwnerCharacter->GetClass()->FindPropertyByName(PropStamina)
		  ->ContainerPtrToValuePtr<float>(OwnerCharacter)
		: nullptr))
	{
		const float NormStamina = FMath::Clamp(*StaminaPtr / 100.f, 0.f, 1.f);
		FatigueAlpha = FMath::InterpEaseIn(0.f, 1.f, 1.f - NormStamina, 2.f);
	}

	// Fear → FearAlpha
	if (float* FearPtr = reinterpret_cast<float*>(
		OwnerCharacter->GetClass()->FindPropertyByName(PropFear)
		? OwnerCharacter->GetClass()->FindPropertyByName(PropFear)
		  ->ContainerPtrToValuePtr<float>(OwnerCharacter)
		: nullptr))
	{
		FearAlpha = FMath::Clamp(*FearPtr / 100.f, 0.f, 1.f);
	}
}
