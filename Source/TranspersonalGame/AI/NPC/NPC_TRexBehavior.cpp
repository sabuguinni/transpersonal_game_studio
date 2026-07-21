#include "NPC_TRexBehavior.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UNPC_TRexBehavior::UNPC_TRexBehavior()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

	// Initialize default values
	CurrentState = ENPC_DinosaurState::Idle;
	PreviousState = ENPC_DinosaurState::Idle;
	
	// Territory settings
	TerritoryCenter = FVector::ZeroVector;
	TerritoryRadius = 5000.0f;
	PatrolRadius = 3000.0f;
	
	// Detection ranges
	DetectionRange = 3000.0f;
	AttackRange = 300.0f;
	ChaseRange = 5000.0f;
	
	// Initialize targeting
	CurrentTarget = nullptr;
	CurrentPatrolPoint = FVector::ZeroVector;
	NextPatrolPoint = FVector::ZeroVector;
	
	// Timing
	StateChangeTime = 0.0f;
	PatrolPointReachThreshold = 500.0f;
	AttackCooldown = 3.0f;
	LastAttackTime = -10.0f;
	
	// Movement speeds
	PatrolSpeed = 200.0f;
	ChaseSpeed = 600.0f;
	AttackSpeed = 400.0f;
}

void UNPC_TRexBehavior::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize territory center to current location if not set
	if (TerritoryCenter.IsZero())
	{
		TerritoryCenter = GetOwner()->GetActorLocation();
	}
	
	// Generate initial patrol point
	NextPatrolPoint = GenerateRandomPatrolPoint();
	CurrentPatrolPoint = GetOwner()->GetActorLocation();
	
	// Start in patrol state
	TransitionToState(ENPC_DinosaurState::Patrolling);
	
	UE_LOG(LogTemp, Warning, TEXT("T-Rex Behavior initialized at territory center: %s"), *TerritoryCenter.ToString());
}

void UNPC_TRexBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!GetOwner())
	{
		return;
	}
	
	// Update behavior based on current state
	UpdateBehaviorState();
	
	// Execute state-specific behavior
	switch (CurrentState)
	{
	case ENPC_DinosaurState::Patrolling:
		HandlePatrolState();
		break;
	case ENPC_DinosaurState::Hunting:
		HandleHuntState();
		break;
	case ENPC_DinosaurState::Attacking:
		HandleAttackState();
		break;
	case ENPC_DinosaurState::Returning:
		HandleReturnState();
		break;
	default:
		break;
	}
}

void UNPC_TRexBehavior::InitializeBehavior(FVector NewTerritoryCenter, float NewTerritoryRadius)
{
	TerritoryCenter = NewTerritoryCenter;
	TerritoryRadius = NewTerritoryRadius;
	PatrolRadius = TerritoryRadius * 0.6f; // Patrol within 60% of territory
	
	// Reset patrol points
	NextPatrolPoint = GenerateRandomPatrolPoint();
	CurrentPatrolPoint = GetOwner()->GetActorLocation();
	
	UE_LOG(LogTemp, Log, TEXT("T-Rex territory initialized: Center=%s, Radius=%.1f"), 
		*TerritoryCenter.ToString(), TerritoryRadius);
}

void UNPC_TRexBehavior::UpdateBehaviorState()
{
	// Check for state transitions
	switch (CurrentState)
	{
	case ENPC_DinosaurState::Patrolling:
		if (ShouldTransitionToHunt())
		{
			TransitionToState(ENPC_DinosaurState::Hunting);
		}
		break;
		
	case ENPC_DinosaurState::Hunting:
		if (ShouldTransitionToAttack())
		{
			TransitionToState(ENPC_DinosaurState::Attacking);
		}
		else if (ShouldTransitionToReturn())
		{
			TransitionToState(ENPC_DinosaurState::Returning);
		}
		break;
		
	case ENPC_DinosaurState::Attacking:
		if (!CanAttackTarget())
		{
			if (CurrentTarget && GetDistanceToTarget() < ChaseRange)
			{
				TransitionToState(ENPC_DinosaurState::Hunting);
			}
			else
			{
				TransitionToState(ENPC_DinosaurState::Returning);
			}
		}
		break;
		
	case ENPC_DinosaurState::Returning:
		if (IsInTerritory())
		{
			TransitionToState(ENPC_DinosaurState::Patrolling);
		}
		break;
	}
}

void UNPC_TRexBehavior::SetTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
	if (NewTarget)
	{
		UE_LOG(LogTemp, Log, TEXT("T-Rex acquired target: %s"), *NewTarget->GetName());
	}
}

void UNPC_TRexBehavior::ClearTarget()
{
	CurrentTarget = nullptr;
	UE_LOG(LogTemp, Log, TEXT("T-Rex target cleared"));
}

float UNPC_TRexBehavior::GetDistanceToTarget() const
{
	if (!CurrentTarget || !GetOwner())
	{
		return -1.0f;
	}
	
	return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool UNPC_TRexBehavior::IsInTerritory() const
{
	if (!GetOwner())
	{
		return false;
	}
	
	float DistanceFromCenter = FVector::Dist(GetOwner()->GetActorLocation(), TerritoryCenter);
	return DistanceFromCenter <= TerritoryRadius;
}

void UNPC_TRexBehavior::HandlePatrolState()
{
	if (!GetOwner())
	{
		return;
	}
	
	FVector CurrentLocation = GetOwner()->GetActorLocation();
	float DistanceToPatrolPoint = FVector::Dist(CurrentLocation, NextPatrolPoint);
	
	// Check if we've reached the patrol point
	if (DistanceToPatrolPoint < PatrolPointReachThreshold)
	{
		// Generate new patrol point
		CurrentPatrolPoint = NextPatrolPoint;
		NextPatrolPoint = GenerateRandomPatrolPoint();
		
		UE_LOG(LogTemp, Log, TEXT("T-Rex reached patrol point, new target: %s"), *NextPatrolPoint.ToString());
	}
	
	// Move towards patrol point
	MoveToLocation(NextPatrolPoint, PatrolSpeed);
}

void UNPC_TRexBehavior::HandleHuntState()
{
	if (!CurrentTarget)
	{
		// Lost target, find new one or return to patrol
		AActor* NewTarget = FindNearestPlayer();
		if (NewTarget)
		{
			SetTarget(NewTarget);
		}
		else
		{
			TransitionToState(ENPC_DinosaurState::Returning);
			return;
		}
	}
	
	// Chase the target
	MoveToLocation(CurrentTarget->GetActorLocation(), ChaseSpeed);
	RotateTowardsTarget(CurrentTarget, GetWorld()->GetDeltaSeconds());
}

void UNPC_TRexBehavior::HandleAttackState()
{
	if (!CurrentTarget)
	{
		TransitionToState(ENPC_DinosaurState::Returning);
		return;
	}
	
	// Face the target
	RotateTowardsTarget(CurrentTarget, GetWorld()->GetDeltaSeconds());
	
	// Check if we can attack
	if (CanAttackTarget())
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastAttackTime >= AttackCooldown)
		{
			// Perform attack
			LastAttackTime = CurrentTime;
			UE_LOG(LogTemp, Warning, TEXT("T-Rex ATTACKS %s!"), *CurrentTarget->GetName());
			
			// Move slightly towards target during attack
			MoveToLocation(CurrentTarget->GetActorLocation(), AttackSpeed);
		}
	}
}

void UNPC_TRexBehavior::HandleReturnState()
{
	// Move back to territory center
	MoveToLocation(TerritoryCenter, ChaseSpeed);
	
	// Clear target when returning
	if (CurrentTarget)
	{
		ClearTarget();
	}
}

FVector UNPC_TRexBehavior::GenerateRandomPatrolPoint()
{
	// Generate random point within patrol radius of territory center
	float RandomAngle = FMath::RandRange(0.0f, 360.0f);
	float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
	
	FVector RandomDirection = FVector(
		FMath::Cos(FMath::DegreesToRadians(RandomAngle)),
		FMath::Sin(FMath::DegreesToRadians(RandomAngle)),
		0.0f
	);
	
	FVector PatrolPoint = TerritoryCenter + (RandomDirection * RandomDistance);
	PatrolPoint.Z = TerritoryCenter.Z; // Keep same height
	
	return PatrolPoint;
}

AActor* UNPC_TRexBehavior::FindNearestPlayer()
{
	if (!GetOwner() || !GetWorld())
	{
		return nullptr;
	}
	
	// Find all player controllers
	TArray<AActor*> PlayerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), PlayerActors);
	
	AActor* NearestPlayer = nullptr;
	float NearestDistance = DetectionRange;
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	
	for (AActor* Player : PlayerActors)
	{
		if (!Player)
		{
			continue;
		}
		
		// Skip if it's the T-Rex itself
		if (Player == GetOwner())
		{
			continue;
		}
		
		float Distance = FVector::Dist(OwnerLocation, Player->GetActorLocation());
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestPlayer = Player;
		}
	}
	
	return NearestPlayer;
}

bool UNPC_TRexBehavior::CanAttackTarget() const
{
	if (!CurrentTarget || !GetOwner())
	{
		return false;
	}
	
	float Distance = GetDistanceToTarget();
	return Distance >= 0.0f && Distance <= AttackRange;
}

void UNPC_TRexBehavior::MoveToLocation(FVector TargetLocation, float Speed)
{
	if (!GetOwner())
	{
		return;
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}
	
	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}
	
	// Set movement speed
	MovementComp->MaxWalkSpeed = Speed;
	
	// Calculate direction to target
	FVector CurrentLocation = GetOwner()->GetActorLocation();
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
	
	// Add movement input
	OwnerCharacter->AddMovementInput(Direction, 1.0f);
}

void UNPC_TRexBehavior::RotateTowardsTarget(AActor* Target, float DeltaTime)
{
	if (!Target || !GetOwner())
	{
		return;
	}
	
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
	
	FRotator TargetRotation = Direction.Rotation();
	FRotator CurrentRotation = GetOwner()->GetActorRotation();
	
	// Smooth rotation
	float RotationSpeed = 180.0f; // degrees per second
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
	
	GetOwner()->SetActorRotation(NewRotation);
}

void UNPC_TRexBehavior::TransitionToState(ENPC_DinosaurState NewState)
{
	if (NewState == CurrentState)
	{
		return;
	}
	
	PreviousState = CurrentState;
	CurrentState = NewState;
	StateChangeTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	
	UE_LOG(LogTemp, Log, TEXT("T-Rex state transition: %d -> %d"), 
		static_cast<int32>(PreviousState), static_cast<int32>(CurrentState));
}

bool UNPC_TRexBehavior::ShouldTransitionToHunt() const
{
	// Look for nearby players
	AActor* NearestPlayer = const_cast<UNPC_TRexBehavior*>(this)->FindNearestPlayer();
	if (NearestPlayer)
	{
		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), NearestPlayer->GetActorLocation());
		return Distance <= DetectionRange;
	}
	
	return false;
}

bool UNPC_TRexBehavior::ShouldTransitionToAttack() const
{
	if (!CurrentTarget)
	{
		return false;
	}
	
	float Distance = GetDistanceToTarget();
	return Distance >= 0.0f && Distance <= AttackRange;
}

bool UNPC_TRexBehavior::ShouldTransitionToReturn() const
{
	// Return if no target or target is too far from territory
	if (!CurrentTarget)
	{
		return !IsInTerritory();
	}
	
	float DistanceToTarget = GetDistanceToTarget();
	if (DistanceToTarget > ChaseRange)
	{
		return true;
	}
	
	// Return if we're too far from territory
	return !IsInTerritory();
}

bool UNPC_TRexBehavior::ShouldTransitionToPatrol() const
{
	// Transition to patrol when back in territory and no immediate threats
	if (!IsInTerritory())
	{
		return false;
	}
	
	// Check if there are any nearby players
	AActor* NearestPlayer = const_cast<UNPC_TRexBehavior*>(this)->FindNearestPlayer();
	if (NearestPlayer)
	{
		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), NearestPlayer->GetActorLocation());
		return Distance > DetectionRange;
	}
	
	return true;
}