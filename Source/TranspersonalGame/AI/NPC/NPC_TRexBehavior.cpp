#include "NPC_TRexBehavior.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ANPC_TRexBehavior::ANPC_TRexBehavior()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize AI Components
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

	// Initialize default values
	CurrentState = ENPC_TRexState::Patrolling;
	TerritoryCenter = FVector::ZeroVector;
	CurrentTarget = nullptr;
	CurrentPatrolIndex = 0;
	StateTimer = 0.0f;
	LastAttackTime = 0.0f;

	// Setup AI Perception
	SetupAIPerception();
}

void ANPC_TRexBehavior::BeginPlay()
{
	Super::BeginPlay();

	// Set territory center to current location if not set
	if (TerritoryCenter.IsZero())
	{
		TerritoryCenter = GetPawn()->GetActorLocation();
	}

	// Generate patrol points
	GeneratePatrolPoints();

	// Setup blackboard
	SetupBlackboard();

	// Start behavior tree if available
	if (TRexBehaviorTree)
	{
		RunBehaviorTree(TRexBehaviorTree);
	}

	// Start patrolling
	StartPatrolling();

	UE_LOG(LogTemp, Warning, TEXT("T-Rex AI Controller initialized at location: %s"), *TerritoryCenter.ToString());
}

void ANPC_TRexBehavior::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	StateTimer += DeltaTime;

	// Update current state behavior
	switch (CurrentState)
	{
		case ENPC_TRexState::Patrolling:
			UpdatePatrolling(DeltaTime);
			break;
		case ENPC_TRexState::Hunting:
			UpdateHunting(DeltaTime);
			break;
		case ENPC_TRexState::Attacking:
			UpdateAttacking(DeltaTime);
			break;
		case ENPC_TRexState::Feeding:
			UpdateFeeding(DeltaTime);
			break;
		case ENPC_TRexState::Resting:
			UpdateResting(DeltaTime);
			break;
	}
}

void ANPC_TRexBehavior::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		// Set territory center to pawn location
		TerritoryCenter = InPawn->GetActorLocation();
		
		// Set movement speed
		if (ACharacter* Character = Cast<ACharacter>(InPawn))
		{
			if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
			{
				MovementComp->MaxWalkSpeed = TRexStats.MovementSpeed;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("T-Rex AI Controller possessed pawn: %s"), *InPawn->GetName());
	}
}

void ANPC_TRexBehavior::SetState(ENPC_TRexState NewState)
{
	if (CurrentState != NewState)
	{
		UE_LOG(LogTemp, Log, TEXT("T-Rex state changed from %d to %d"), (int32)CurrentState, (int32)NewState);
		
		CurrentState = NewState;
		StateTimer = 0.0f;

		// Update blackboard
		if (BlackboardComponent)
		{
			BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), (uint8)CurrentState);
		}
	}
}

void ANPC_TRexBehavior::StartPatrolling()
{
	SetState(ENPC_TRexState::Patrolling);
	SetMovementSpeed(TRexStats.MovementSpeed);
	
	if (PatrolPoints.Num() > 0)
	{
		MoveToNextPatrolPoint();
	}

	UE_LOG(LogTemp, Log, TEXT("T-Rex started patrolling"));
}

void ANPC_TRexBehavior::StartHunting(AActor* Target)
{
	if (Target)
	{
		SetState(ENPC_TRexState::Hunting);
		CurrentTarget = Target;
		SetMovementSpeed(TRexStats.HuntingSpeed);

		// Update blackboard
		if (BlackboardComponent)
		{
			BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
		}

		// Move to target
		MoveToActor(Target, TRexStats.AttackRange);

		UE_LOG(LogTemp, Warning, TEXT("T-Rex started hunting target: %s"), *Target->GetName());
	}
}

void ANPC_TRexBehavior::StartAttacking()
{
	SetState(ENPC_TRexState::Attacking);
	
	// Stop movement during attack
	StopMovement();

	UE_LOG(LogTemp, Warning, TEXT("T-Rex started attacking"));
}

void ANPC_TRexBehavior::StartFeeding()
{
	SetState(ENPC_TRexState::Feeding);
	
	// Stop movement during feeding
	StopMovement();

	UE_LOG(LogTemp, Log, TEXT("T-Rex started feeding"));
}

void ANPC_TRexBehavior::StartResting()
{
	SetState(ENPC_TRexState::Resting);
	
	// Stop movement during rest
	StopMovement();

	UE_LOG(LogTemp, Log, TEXT("T-Rex started resting"));
}

void ANPC_TRexBehavior::SetTerritoryCenter(FVector NewCenter)
{
	TerritoryCenter = NewCenter;
	GeneratePatrolPoints();

	UE_LOG(LogTemp, Log, TEXT("T-Rex territory center set to: %s"), *NewCenter.ToString());
}

void ANPC_TRexBehavior::GeneratePatrolPoints()
{
	PatrolPoints.Empty();

	// Generate 6 patrol points in a circle around territory center
	const int32 NumPoints = 6;
	const float AngleStep = 360.0f / NumPoints;

	for (int32 i = 0; i < NumPoints; i++)
	{
		float Angle = FMath::DegreesToRadians(AngleStep * i);
		float X = TerritoryCenter.X + TRexStats.PatrolRadius * FMath::Cos(Angle);
		float Y = TerritoryCenter.Y + TRexStats.PatrolRadius * FMath::Sin(Angle);
		float Z = TerritoryCenter.Z;

		PatrolPoints.Add(FVector(X, Y, Z));
	}

	CurrentPatrolIndex = 0;

	UE_LOG(LogTemp, Log, TEXT("Generated %d patrol points for T-Rex"), PatrolPoints.Num());
}

bool ANPC_TRexBehavior::IsInTerritory(FVector Location) const
{
	float Distance = FVector::Dist(Location, TerritoryCenter);
	return Distance <= TRexStats.PatrolRadius;
}

AActor* ANPC_TRexBehavior::FindNearestPrey()
{
	if (!GetPawn())
		return nullptr;

	FVector PawnLocation = GetPawn()->GetActorLocation();
	AActor* NearestPrey = nullptr;
	float NearestDistance = TRexStats.DetectionRange;

	// Find all characters in the world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor && Actor != GetPawn())
		{
			float Distance = FVector::Dist(PawnLocation, Actor->GetActorLocation());
			
			if (Distance < NearestDistance && CanSeeTarget(Actor))
			{
				NearestDistance = Distance;
				NearestPrey = Actor;
			}
		}
	}

	return NearestPrey;
}

float ANPC_TRexBehavior::GetDistanceToTarget(AActor* Target) const
{
	if (!Target || !GetPawn())
		return -1.0f;

	return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

bool ANPC_TRexBehavior::CanSeeTarget(AActor* Target) const
{
	if (!Target || !GetPawn())
		return false;

	// Simple line trace to check visibility
	FVector Start = GetPawn()->GetActorLocation();
	FVector End = Target->GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetPawn());

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
	
	return !bHit || HitResult.GetActor() == Target;
}

void ANPC_TRexBehavior::PerformAttack()
{
	if (!CurrentTarget)
		return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	
	// Check attack cooldown
	if (CurrentTime - LastAttackTime < 2.0f)
		return;

	LastAttackTime = CurrentTime;

	// Deal damage to target
	UE_LOG(LogTemp, Warning, TEXT("T-Rex attacks %s for %.1f damage"), 
		*CurrentTarget->GetName(), TRexStats.AttackDamage);

	// After attack, start feeding if target is defeated
	StartFeeding();
}

bool ANPC_TRexBehavior::IsInAttackRange(AActor* Target) const
{
	if (!Target)
		return false;

	float Distance = GetDistanceToTarget(Target);
	return Distance > 0 && Distance <= TRexStats.AttackRange;
}

void ANPC_TRexBehavior::MoveToNextPatrolPoint()
{
	if (PatrolPoints.Num() == 0)
		return;

	FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];
	MoveToLocation(TargetPoint, 100.0f);

	// Move to next patrol point
	CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();

	UE_LOG(LogTemp, Log, TEXT("T-Rex moving to patrol point: %s"), *TargetPoint.ToString());
}

void ANPC_TRexBehavior::SetMovementSpeed(float Speed)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (ACharacter* Character = Cast<ACharacter>(ControlledPawn))
		{
			if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
			{
				MovementComp->MaxWalkSpeed = Speed;
			}
		}
	}
}

void ANPC_TRexBehavior::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	// Handle perception updates
	for (AActor* Actor : UpdatedActors)
	{
		if (Actor && Actor != GetPawn())
		{
			// If we're patrolling and detect prey, start hunting
			if (CurrentState == ENPC_TRexState::Patrolling)
			{
				if (ACharacter* Character = Cast<ACharacter>(Actor))
				{
					float Distance = GetDistanceToTarget(Actor);
					if (Distance <= TRexStats.DetectionRange)
					{
						StartHunting(Actor);
						break;
					}
				}
			}
		}
	}
}

void ANPC_TRexBehavior::UpdatePatrolling(float DeltaTime)
{
	// Look for prey while patrolling
	AActor* Prey = FindNearestPrey();
	if (Prey)
	{
		StartHunting(Prey);
		return;
	}

	// Check if we need to rest
	if (ShouldRest())
	{
		StartResting();
		return;
	}

	// Continue patrolling if we've reached current patrol point
	if (GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		MoveToNextPatrolPoint();
	}
}

void ANPC_TRexBehavior::UpdateHunting(float DeltaTime)
{
	if (!CurrentTarget)
	{
		StartPatrolling();
		return;
	}

	float Distance = GetDistanceToTarget(CurrentTarget);
	
	// Check if target is in attack range
	if (IsInAttackRange(CurrentTarget))
	{
		StartAttacking();
		return;
	}

	// Check if target is too far away or out of territory
	if (Distance > TRexStats.DetectionRange * 1.5f || !IsInTerritory(CurrentTarget->GetActorLocation()))
	{
		CurrentTarget = nullptr;
		StartPatrolling();
		return;
	}

	// Continue chasing target
	MoveToActor(CurrentTarget, TRexStats.AttackRange);
}

void ANPC_TRexBehavior::UpdateAttacking(float DeltaTime)
{
	if (!CurrentTarget)
	{
		StartPatrolling();
		return;
	}

	// Check if still in attack range
	if (!IsInAttackRange(CurrentTarget))
	{
		StartHunting(CurrentTarget);
		return;
	}

	// Perform attack
	PerformAttack();
}

void ANPC_TRexBehavior::UpdateFeeding(float DeltaTime)
{
	// Feed for specified duration
	if (StateTimer >= TRexStats.FeedingDuration)
	{
		CurrentTarget = nullptr;
		StartResting();
	}
}

void ANPC_TRexBehavior::UpdateResting(float DeltaTime)
{
	// Rest for specified duration
	if (StateTimer >= TRexStats.RestDuration)
	{
		StartPatrolling();
	}
}

FVector ANPC_TRexBehavior::GetRandomPatrolPoint() const
{
	if (PatrolPoints.Num() == 0)
		return TerritoryCenter;

	int32 RandomIndex = FMath::RandRange(0, PatrolPoints.Num() - 1);
	return PatrolPoints[RandomIndex];
}

bool ANPC_TRexBehavior::ShouldRest() const
{
	// Rest every 5 minutes of patrolling
	return StateTimer > 300.0f;
}

bool ANPC_TRexBehavior::ShouldFeed() const
{
	// Feed after successful attack
	return CurrentState == ENPC_TRexState::Attacking;
}

void ANPC_TRexBehavior::SetupAIPerception()
{
	if (!AIPerceptionComponent)
		return;

	// Setup sight sense
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = TRexStats.DetectionRange;
		SightConfig->LoseSightRadius = TRexStats.DetectionRange * 1.2f;
		SightConfig->PeripheralVisionAngleDegrees = 120.0f;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;

		AIPerceptionComponent->ConfigureSense(*SightConfig);
	}

	// Setup hearing sense
	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = TRexStats.DetectionRange * 0.8f;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

		AIPerceptionComponent->ConfigureSense(*HearingConfig);
	}

	// Set dominant sense
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	// Bind perception events
	AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_TRexBehavior::OnPerceptionUpdated);
}

void ANPC_TRexBehavior::SetupBlackboard()
{
	if (!BlackboardComponent)
		return;

	// Set initial blackboard values
	BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), (uint8)CurrentState);
	BlackboardComponent->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
	BlackboardComponent->SetValueAsFloat(TEXT("PatrolRadius"), TRexStats.PatrolRadius);
	BlackboardComponent->SetValueAsFloat(TEXT("DetectionRange"), TRexStats.DetectionRange);
	BlackboardComponent->SetValueAsFloat(TEXT("AttackRange"), TRexStats.AttackRange);

	UE_LOG(LogTemp, Log, TEXT("T-Rex blackboard initialized"));
}