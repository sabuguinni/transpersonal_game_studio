#include "DinoSurvivorAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "../TranspersonalCharacter.h"

UDinoSurvivorAnimInstance::UDinoSurvivorAnimInstance()
{
	// Locomotion defaults
	GroundSpeed        = 0.f;
	MovementDirection  = 0.f;
	bIsMoving          = false;
	bIsInAir           = false;
	bIsCrouching       = false;
	bIsSprinting       = false;

	// Survival defaults
	Stamina            = 100.f;
	Health             = 100.f;
	bIsLimping         = false;
	FearLevel          = 0.f;

	// Combat defaults
	bIsArmed           = false;
	bIsAttacking       = false;

	// Config thresholds
	SprintThreshold    = 450.f;   // cm/s — above walk speed (300)
	LimpHealthThreshold = 25.f;
	MinMoveSpeed       = 10.f;

	CachedMovementComp = nullptr;
	CachedCharacter    = nullptr;
}

void UDinoSurvivorAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Owner = TryGetPawnOwner();
	if (!Owner) return;

	CachedCharacter    = Cast<ATranspersonalCharacter>(Owner);
	CachedMovementComp = Cast<UCharacterMovementComponent>(
		Owner->GetMovementComponent());
}

void UDinoSurvivorAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedMovementComp || !CachedCharacter) return;

	// ── Locomotion ──────────────────────────────────────────────────────────
	const FVector Velocity    = CachedMovementComp->Velocity;
	const FVector HorizVel    = FVector(Velocity.X, Velocity.Y, 0.f);
	GroundSpeed               = HorizVel.Size();
	bIsMoving                 = GroundSpeed > MinMoveSpeed;
	bIsInAir                  = CachedMovementComp->IsFalling();
	bIsCrouching              = CachedCharacter->bIsCrouched;
	bIsSprinting              = GroundSpeed > SprintThreshold;

	// Movement direction relative to actor forward (-180..180)
	if (bIsMoving)
	{
		const FRotator ActorRot   = CachedCharacter->GetActorRotation();
		const FRotator VelRot     = UKismetMathLibrary::MakeRotFromX(HorizVel);
		const FRotator DeltaRot   = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot);
		MovementDirection         = FMath::Clamp(DeltaRot.Yaw, -180.f, 180.f);
	}
	else
	{
		MovementDirection = 0.f;
	}

	// ── Survival State ───────────────────────────────────────────────────────
	// Read survival stats from TranspersonalCharacter if available
	// Properties are exposed as UPROPERTY floats on the character
	Health    = CachedCharacter->GetHealth();
	Stamina   = CachedCharacter->GetStamina();
	FearLevel = CachedCharacter->GetFear();

	bIsLimping = Health < LimpHealthThreshold;

	// ── Combat ───────────────────────────────────────────────────────────────
	// bIsArmed and bIsAttacking are set externally by the combat system
	// via Blueprint or C++ calls — no polling needed here
}
