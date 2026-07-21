#include "Combat_DinosaurCombatSystem.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

ACombat_DinosaurCombatSystem::ACombat_DinosaurCombatSystem()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create detection sphere component
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	RootComponent = DetectionSphere;
	DetectionSphere->SetSphereRadius(5000.0f);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	DetectionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Initialize combat state
	CurrentCombatState = ECombat_DinosaurCombatState::Idle;
	CurrentTarget = nullptr;
	bIsAggressive = true;
	bCanFlee = false;
	FleeHealthThreshold = 20.0f;
	LastAttackTime = 0.0f;
	AttackCooldown = 2.0f;

	// Initialize combat stats with default values
	CombatStats = FCombat_DinosaurCombatStats();
}

void ACombat_DinosaurCombatSystem::BeginPlay()
{
	Super::BeginPlay();
	
	// Set detection sphere radius from combat stats
	if (DetectionSphere)
	{
		DetectionSphere->SetSphereRadius(CombatStats.DetectionRadius);
	}

	// Start combat behavior updates
	GetWorldTimerManager().SetTimer(AttackCooldownTimer, this, &ACombat_DinosaurCombatSystem::UpdateCombatBehavior, 0.5f, true);
}

void ACombat_DinosaurCombatSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update combat behavior based on current state
	switch (CurrentCombatState)
	{
		case ECombat_DinosaurCombatState::Hunting:
			ProcessHuntingState();
			break;
		case ECombat_DinosaurCombatState::Attacking:
			ProcessAttackingState();
			break;
		case ECombat_DinosaurCombatState::Fleeing:
			ProcessFleeingState();
			break;
		default:
			break;
	}
}

void ACombat_DinosaurCombatSystem::StartHunting(AActor* Target)
{
	if (!Target || !bIsAggressive)
	{
		return;
	}

	CurrentTarget = Target;
	SetCombatState(ECombat_DinosaurCombatState::Hunting);
	OnTargetAcquired(Target);

	UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatSystem: Started hunting target %s"), *Target->GetName());
}

void ACombat_DinosaurCombatSystem::AttackTarget()
{
	if (!CurrentTarget || !CanAttack())
	{
		return;
	}

	if (IsTargetInAttackRange())
	{
		// Execute attack
		SetCombatState(ECombat_DinosaurCombatState::Attacking);
		LastAttackTime = GetWorld()->GetTimeSeconds();

		// Apply damage to target if it's a character
		if (ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget))
		{
			// In a full implementation, this would call a damage system
			UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatSystem: Attacking %s for %f damage"), 
				*CurrentTarget->GetName(), CombatStats.AttackDamage);
		}

		OnAttackExecuted(CurrentTarget);

		// Set cooldown timer
		GetWorldTimerManager().SetTimer(AttackCooldownTimer, [this]()
		{
			if (CurrentCombatState == ECombat_DinosaurCombatState::Attacking)
			{
				SetCombatState(ECombat_DinosaurCombatState::Hunting);
			}
		}, AttackCooldown, false);
	}
}

void ACombat_DinosaurCombatSystem::FleeFromThreat()
{
	if (!bCanFlee)
	{
		return;
	}

	SetCombatState(ECombat_DinosaurCombatState::Fleeing);
	CurrentTarget = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatSystem: Fleeing from threat"));

	// Set timer to return to idle state after fleeing
	GetWorldTimerManager().SetTimer(AttackCooldownTimer, [this]()
	{
		SetCombatState(ECombat_DinosaurCombatState::Idle);
	}, 5.0f, false);
}

void ACombat_DinosaurCombatSystem::TakeDamage(float DamageAmount, AActor* DamageSource)
{
	CombatStats.Health = FMath::Max(0.0f, CombatStats.Health - DamageAmount);

	UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatSystem: Took %f damage, health now %f"), 
		DamageAmount, CombatStats.Health);

	// Check if should flee
	if (bCanFlee && CombatStats.Health <= FleeHealthThreshold)
	{
		FleeFromThreat();
	}
	// If not already targeting the damage source, switch targets
	else if (DamageSource && DamageSource != CurrentTarget && bIsAggressive)
	{
		StartHunting(DamageSource);
	}

	// Check for death
	if (CombatStats.Health <= 0.0f)
	{
		SetCombatState(ECombat_DinosaurCombatState::Idle);
		CurrentTarget = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatSystem: Dinosaur defeated"));
	}
}

AActor* ACombat_DinosaurCombatSystem::DetectNearbyPlayer()
{
	if (!GetWorld())
	{
		return nullptr;
	}

	// Find all characters in detection range
	TArray<AActor*> OverlappingActors;
	DetectionSphere->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	// Return the closest character
	AActor* ClosestTarget = nullptr;
	float ClosestDistance = CombatStats.DetectionRadius;

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor != this->GetOwner())
		{
			float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestTarget = Actor;
			}
		}
	}

	return ClosestTarget;
}

bool ACombat_DinosaurCombatSystem::IsTargetInAttackRange()
{
	if (!CurrentTarget)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	return Distance <= CombatStats.AttackRange;
}

void ACombat_DinosaurCombatSystem::SetCombatState(ECombat_DinosaurCombatState NewState)
{
	ECombat_DinosaurCombatState OldState = CurrentCombatState;
	CurrentCombatState = NewState;

	OnCombatStateChanged(OldState, NewState);

	UE_LOG(LogTemp, Log, TEXT("DinosaurCombatSystem: Combat state changed from %d to %d"), 
		(int32)OldState, (int32)NewState);
}

void ACombat_DinosaurCombatSystem::UpdateCombatBehavior()
{
	if (CurrentCombatState == ECombat_DinosaurCombatState::Idle && bIsAggressive)
	{
		// Look for nearby players to hunt
		AActor* NearbyPlayer = DetectNearbyPlayer();
		if (NearbyPlayer)
		{
			StartHunting(NearbyPlayer);
		}
	}
}

void ACombat_DinosaurCombatSystem::ProcessHuntingState()
{
	if (!CurrentTarget)
	{
		SetCombatState(ECombat_DinosaurCombatState::Idle);
		return;
	}

	// Check if target is still in detection range
	float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (DistanceToTarget > CombatStats.DetectionRadius)
	{
		CurrentTarget = nullptr;
		SetCombatState(ECombat_DinosaurCombatState::Idle);
		return;
	}

	// Check if target is in attack range
	if (IsTargetInAttackRange() && CanAttack())
	{
		AttackTarget();
	}
}

void ACombat_DinosaurCombatSystem::ProcessAttackingState()
{
	// Attack state is handled by timer in AttackTarget function
	// This function can be used for attack animations or effects
}

void ACombat_DinosaurCombatSystem::ProcessFleeingState()
{
	// Fleeing behavior - move away from threats
	// In a full implementation, this would control movement away from the threat
}

bool ACombat_DinosaurCombatSystem::CanAttack() const
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	return (CurrentTime - LastAttackTime) >= AttackCooldown;
}