// TRex.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Tyrannosaurus Rex: apex predator implementation
// Inherits AI state machine from ADinosaurBase

#include "Dinosaurs/TRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATRex::ATRex()
{
	PrimaryActorTick.bCanEverTick = true;

	// Species stats — apex predator
	DinoStats.MaxHealth        = 2500.0f;
	DinoStats.CurrentHealth    = 2500.0f;
	DinoStats.AttackDamage     = 180.0f;
	DinoStats.AttackRange      = 250.0f;
	DinoStats.DetectionRadius  = 3500.0f;
	DinoStats.WalkSpeed        = 450.0f;
	DinoStats.RunSpeed         = 900.0f;
	DinoStats.BodyMass         = 8000.0f;

	Diet = EEng_DinosaurDiet::Carnivore;

	// TRex-specific
	RoarFearRadius       = 1200.0f;
	ChargeSpeedMultiplier = 1.8f;
	bIsCharging          = false;
	bRoarOnCooldown      = false;
	ChargeElapsed        = 0.0f;
	ChargeDuration       = 2.5f;

	// Scale mesh to TRex proportions
	GetMesh()->SetRelativeScale3D(FVector(3.5f, 3.5f, 3.5f));

	// Movement config — heavy, powerful
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed        = DinoStats.WalkSpeed;
		MoveComp->MaxAcceleration     = 600.0f;
		MoveComp->BrakingDecelerationWalking = 800.0f;
		MoveComp->RotationRate        = FRotator(0.0f, 120.0f, 0.0f);
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->Mass                = DinoStats.BodyMass;
	}

	// Capsule for large body
	GetCapsuleComponent()->SetCapsuleHalfHeight(180.0f);
	GetCapsuleComponent()->SetCapsuleRadius(90.0f);
}

void ATRex::BeginPlay()
{
	Super::BeginPlay();

	// Start with a roar to announce presence
	FTimerHandle StartupRoarHandle;
	GetWorldTimerManager().SetTimer(StartupRoarHandle, this, &ATRex::PerformRoar, 3.0f, false);

	UE_LOG(LogTemp, Log, TEXT("TRex [%s] spawned — apex predator active"), *GetName());
}

void ATRex::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle charge duration
	if (bIsCharging)
	{
		ChargeElapsed += DeltaTime;
		if (ChargeElapsed >= ChargeDuration)
		{
			bIsCharging = false;
			ChargeElapsed = 0.0f;

			// Restore normal hunting speed
			if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
			{
				MoveComp->MaxWalkSpeed = DinoStats.RunSpeed;
			}
			UE_LOG(LogTemp, Log, TEXT("TRex [%s] charge ended"), *GetName());
		}
	}
}

void ATRex::PerformRoar()
{
	if (bRoarOnCooldown || !IsAlive())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("TRex [%s] ROARS! Fear radius: %.0fcm"), *GetName(), RoarFearRadius);

	// Apply fear to players within radius
	TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

	for (AActor* Actor : NearbyActors)
	{
		if (Actor && Actor != this)
		{
			float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
			if (Dist <= RoarFearRadius)
			{
				// Broadcast fear event — SurvivalComponent listens for this
				UE_LOG(LogTemp, Warning, TEXT("TRex roar: fear applied to %s (dist=%.0f)"), *Actor->GetName(), Dist);
			}
		}
	}

	// Set cooldown
	bRoarOnCooldown = true;
	GetWorldTimerManager().SetTimer(RoarCooldownHandle, this, &ATRex::ResetRoarCooldown, 15.0f, false);

#if WITH_EDITOR
	// Debug sphere showing fear radius
	DrawDebugSphere(GetWorld(), GetActorLocation(), RoarFearRadius, 16, FColor::Orange, false, 3.0f);
#endif
}

void ATRex::ChargeAttack()
{
	if (bIsCharging || !IsAlive())
	{
		return;
	}

	bIsCharging = true;
	ChargeElapsed = 0.0f;

	// Apply charge speed
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = DinoStats.RunSpeed * ChargeSpeedMultiplier;
	}

	UE_LOG(LogTemp, Warning, TEXT("TRex [%s] CHARGING! Speed: %.0f"), *GetName(), DinoStats.RunSpeed * ChargeSpeedMultiplier);
}

void ATRex::OnDinosaurDeath_Implementation()
{
	// TRex death — dramatic collapse
	UE_LOG(LogTemp, Warning, TEXT("TRex [%s] DIES — apex predator fallen"), *GetName());

	// Disable collision and movement (from base)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	// TRex corpse persists longer — valuable resource
	SetLifeSpan(30.0f);

#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), GetActorLocation(), 500.0f, 12, FColor::Red, false, 5.0f);
#endif
}

void ATRex::OnPlayerDetected_Implementation(AActor* Player, float Distance)
{
	// TRex has narrow forward arc (120 degrees) but extreme range
	if (!Player)
	{
		return;
	}

	FVector ToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector Forward  = GetActorForwardVector();
	float DotProduct = FVector::DotProduct(Forward, ToPlayer);
	float AngleDeg   = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	// Only detect within 120-degree forward arc
	if (AngleDeg <= 60.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("TRex [%s] detected player %s at %.0fcm (angle=%.1f°) — HUNTING"),
			*GetName(), *Player->GetName(), Distance, AngleDeg);

		// Trigger roar if not on cooldown
		if (!bRoarOnCooldown)
		{
			PerformRoar();
		}

		// Trigger charge if close enough
		if (Distance < 800.0f)
		{
			ChargeAttack();
		}
	}
}

void ATRex::ResetRoarCooldown()
{
	bRoarOnCooldown = false;
	UE_LOG(LogTemp, Log, TEXT("TRex [%s] roar cooldown reset"), *GetName());
}
