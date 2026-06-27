#include "TyrannosaurusRex.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATyrannosaurusRex::ATyrannosaurusRex()
{
	// Override base stats for apex predator
	Stats.MaxHealth = 500.0f;
	Stats.CurrentHealth = 500.0f;
	Stats.AttackDamage = 80.0f;
	Stats.DetectionRadius = 2500.0f;
	Stats.AttackRadius = 300.0f;
	Stats.MoveSpeed = 350.0f;
	Stats.SprintSpeed = 700.0f;
	Stats.bIsCarnivore = true;
	Stats.bIsPackHunter = false;
	Stats.Species = EDinosaurSpecies::TyrannosaurusRex;

	// Large capsule for a 12m tall predator
	GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
	GetCapsuleComponent()->SetCapsuleRadius(80.0f);

	// Scale mesh to approximate T-Rex proportions
	GetMesh()->SetRelativeScale3D(FVector(3.0f, 1.5f, 4.0f));

	// Ability defaults
	RoarStunDuration = 2.5f;
	ChargeSpeedMultiplier = 2.5f;
	ChargeBonusDamage = 50.0f;
	RoarRadius = 1200.0f;

	bIsCharging = false;
	ChargeTimer = 0.0f;

	// Movement tuning
	GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 60.0f, 0.0f); // Slow turn rate
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ATyrannosaurusRex::BeginPlay()
{
	Super::BeginPlay();
	// Start in idle state
	SetBehaviorState(EDinosaurBehaviorState::Idle);
}

void ATyrannosaurusRex::PerformAttack(AActor* Target)
{
	if (!Target || !IsAlive())
	{
		return;
	}

	float TotalDamage = Stats.AttackDamage;

	// Charge bonus damage
	if (bIsCharging)
	{
		TotalDamage += ChargeBonusDamage;
		bIsCharging = false;
		GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
		UE_LOG(LogTemp, Log, TEXT("TRex charge connected! Bonus damage applied. Total: %.1f"), TotalDamage);
	}

	// Apply damage via UE5 damage system
	UGameplayStatics::ApplyDamage(Target, TotalDamage, GetController(), this, UDamageType::StaticClass());

	UE_LOG(LogTemp, Log, TEXT("TRex attacked %s for %.1f damage"), *Target->GetName(), TotalDamage);
}

void ATyrannosaurusRex::PerformRoar()
{
	if (!IsAlive())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("TRex ROARS! Radius: %.0f, StunDuration: %.1fs"), RoarRadius, RoarStunDuration);

	// Find all pawns in roar radius and apply stun (fear/flee response)
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(RoarRadius),
		QueryParams
	);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* NearbyActor = Overlap.GetActor();
		if (NearbyActor && NearbyActor != this)
		{
			// Broadcast roar event — other systems (AI, player fear) can respond
			UE_LOG(LogTemp, Log, TEXT("TRex roar affects: %s"), *NearbyActor->GetName());
		}
	}

#if WITH_EDITOR
	DrawDebugSphere(GetWorld(), GetActorLocation(), RoarRadius, 16, FColor::Orange, false, 2.0f);
#endif
}

void ATyrannosaurusRex::StartCharge()
{
	if (!IsAlive() || bIsCharging || !CurrentTarget)
	{
		return;
	}

	bIsCharging = true;
	ChargeTimer = 0.0f;
	GetCharacterMovement()->MaxWalkSpeed = Stats.SprintSpeed * ChargeSpeedMultiplier;
	SetBehaviorState(EDinosaurBehaviorState::Attacking);

	UE_LOG(LogTemp, Log, TEXT("TRex begins charge! Speed: %.0f"), GetCharacterMovement()->MaxWalkSpeed);
}

void ATyrannosaurusRex::OnDeath()
{
	// Cancel any active charge
	bIsCharging = false;
	GetCharacterMovement()->MaxWalkSpeed = 0.0f;

	// Call base death logic (ragdoll, loot, etc.)
	Super::OnDeath();

	UE_LOG(LogTemp, Log, TEXT("TyrannosaurusRex has died at %s"), *GetActorLocation().ToString());
}
