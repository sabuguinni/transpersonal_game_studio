// TRexCharacter.cpp
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260620_003
// Tyrannosaurus Rex full implementation

#include "Dinosaurs/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
	// --- Identity ---
	SpeciesName = FName("Tyrannosaurus Rex");
	Diet = EDinoDiet::Carnivore;
	SizeClass = EDinoSize::Huge;

	// --- Health & combat ---
	MaxHealth = 1200.0f;
	CurrentHealth = 1200.0f;
	AttackDamage = 150.0f;
	AttackRange = 280.0f;       // Long reach — massive jaw
	AttackCooldown = 2.5f;      // Slow but devastating

	// --- Movement ---
	WalkSpeed = 250.0f;
	RunSpeed = 750.0f;
	TurnRate = 60.0f;           // Slow turning — gameplay mechanic (run around it)
	MaxTurnRateDegPerSec = 60.0f;

	// --- Senses ---
	DetectionRadius = 4000.0f;
	FieldOfViewDegrees = 90.0f; // Narrow FOV — forward-facing eyes
	HearingRadius = 3500.0f;    // Exceptional hearing compensates for vision

	// --- Stamina ---
	MaxStamina = 800.0f;
	CurrentStamina = 800.0f;
	StaminaDrainRate = 12.0f;   // Drains fast — TRex can't sustain a long chase
	StaminaRecoveryRate = 20.0f;

	// --- Roar ability ---
	RoarStunRadius = 1200.0f;
	RoarStunDuration = 1.5f;
	RoarCooldown = 15.0f;
	bRoarOnCooldown = false;

	// --- Stomp ---
	StompRadius = 400.0f;
	StompDamage = 80.0f;

	// --- Capsule size — massive creature ---
	GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
	GetCapsuleComponent()->SetCapsuleRadius(80.0f);

	// --- Movement component tuning ---
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
		MoveComp->MaxAcceleration = 400.0f;     // Slow to accelerate
		MoveComp->BrakingDecelerationWalking = 300.0f;
		MoveComp->RotationRate = FRotator(0.0f, MaxTurnRateDegPerSec, 0.0f);
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->GravityScale = 1.2f;          // Heavy — falls faster
		MoveComp->JumpZVelocity = 0.0f;         // Cannot jump
		MoveComp->NavAgentProps.AgentRadius = 80.0f;
		MoveComp->NavAgentProps.AgentHeight = 400.0f;
	}
}

void ATRexCharacter::BeginPlay()
{
	Super::BeginPlay();

	// TRex starts in Idle — will begin wandering after base class timer fires
	UE_LOG(LogTemp, Log, TEXT("TRex [%s] spawned at %s"),
		*GetActorLabel(),
		*GetActorLocation().ToString());
}

void ATRexCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Enforce max turn rate — TRex cannot spin quickly
	// The CharacterMovementComponent RotationRate handles this, but we clamp
	// yaw delta per frame as an extra safety for AI-driven rotations
	// (AI controllers can sometimes snap rotation instantly)
	if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround())
	{
		FRotator CurrentRot = GetActorRotation();
		FRotator DesiredRot = GetCharacterMovement()->GetLastUpdateRotation();
		float YawDelta = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, DesiredRot.Yaw);
		float MaxYawThisFrame = MaxTurnRateDegPerSec * DeltaTime;
		float ClampedYaw = FMath::Clamp(YawDelta, -MaxYawThisFrame, MaxYawThisFrame);
		// Applied via RotationRate in movement component — this is informational
		(void)ClampedYaw; // Actual clamping done by RotationRate property
	}
}

void ATRexCharacter::OnTargetDetected(AActor* Target)
{
	// Call base — sets AIState to Alerted
	Super::OnTargetDetected(Target);

	// First aggro: roar to signal dominance and alert nearby dinos
	if (!bRoarOnCooldown)
	{
		PerformRoar();
	}
}

void ATRexCharacter::PerformRoar()
{
	if (bRoarOnCooldown) return;

	bRoarOnCooldown = true;

	UE_LOG(LogTemp, Log, TEXT("TRex [%s] ROARS — stun radius %.0f cm"),
		*GetActorLabel(), RoarStunRadius);

	// Find all actors in roar radius
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

	FVector MyLoc = GetActorLocation();

	for (AActor* Actor : NearbyActors)
	{
		if (!Actor || Actor == this) continue;

		float Dist = FVector::Dist(MyLoc, Actor->GetActorLocation());
		if (Dist <= RoarStunRadius)
		{
			// Apply stun via gameplay tag or custom interface
			// For now: log the stun target — full stun system in SurvivalComponent
			UE_LOG(LogTemp, Verbose, TEXT("TRex roar stuns: %s (dist=%.0f)"),
				*Actor->GetActorLabel(), Dist);

			// Notify other dinos — they enter Alerted state
			ADinosaurBase* OtherDino = Cast<ADinosaurBase>(Actor);
			if (OtherDino && OtherDino->AIState == EDinoAIState::Idle)
			{
				OtherDino->AIState = EDinoAIState::Alerted;
			}
		}
	}

	// Debug sphere in editor
#if WITH_EDITOR
	DrawDebugSphere(World, MyLoc, RoarStunRadius, 16, FColor::Red, false, 3.0f);
#endif

	// Start cooldown timer
	World->GetTimerManager().SetTimer(
		RoarCooldownTimer,
		this,
		&ATRexCharacter::OnRoarCooldownExpired,
		RoarCooldown,
		false
	);
}

void ATRexCharacter::OnRoarCooldownExpired()
{
	bRoarOnCooldown = false;
	UE_LOG(LogTemp, Verbose, TEXT("TRex [%s] roar ready"), *GetActorLabel());
}

void ATRexCharacter::PerformStomp()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FVector StompLoc = GetActorLocation();
	StompLoc.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); // Ground level

	UE_LOG(LogTemp, Log, TEXT("TRex [%s] STOMP at %s — radius %.0f, dmg %.0f"),
		*GetActorLabel(), *StompLoc.ToString(), StompRadius, StompDamage);

	// AoE damage sphere
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	UGameplayStatics::ApplyRadialDamage(
		World,
		StompDamage,
		StompLoc,
		StompRadius,
		UDamageType::StaticClass(),
		IgnoredActors,
		this,
		GetController(),
		true // Full damage at centre, falls off at edge
	);

#if WITH_EDITOR
	DrawDebugSphere(World, StompLoc, StompRadius, 12, FColor::Orange, false, 2.0f);
#endif
}
