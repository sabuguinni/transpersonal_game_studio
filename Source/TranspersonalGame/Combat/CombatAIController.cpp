#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Combat_TacticalBehaviorComponent.h"
#include "Combat_PackCoordinationComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ACombatAIController::ACombatAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create AI Perception Component
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	// Create Behavior Tree Component
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

	// Create Blackboard Component
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

	// Create Custom Combat Components
	TacticalBehaviorComponent = CreateDefaultSubobject<UCombat_TacticalBehaviorComponent>(TEXT("TacticalBehaviorComponent"));
	PackCoordinationComponent = CreateDefaultSubobject<UCombat_PackCoordinationComponent>(TEXT("PackCoordinationComponent"));

	// Default AI Configuration
	AIPersonality = ECombat_AIPersonality::Aggressive;
	TacticalRole = ECombat_TacticalRole::Flanker;
	AggressionLevel = 0.7f;
	CautiousLevel = 0.3f;
	PackLoyalty = 0.8f;

	// Default Perception Settings
	SightRadius = 1500.0f;
	LoseSightRadius = 1600.0f;
	PeripheralVisionAngleDegrees = 90.0f;
	HearingRange = 1200.0f;

	// Initialize State
	CurrentAIState = ECombat_AIState::Idle;
	CurrentTarget = nullptr;
	PackLeader = nullptr;
	StateTimer = 0.0f;
	LastTargetUpdateTime = 0.0f;
	bHasValidTarget = false;
}

void ACombatAIController::BeginPlay()
{
	Super::BeginPlay();

	SetupPerception();

	// Bind perception events
	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
	}

	// Start behavior tree if available
	if (BehaviorTreeAsset && BlackboardAsset)
	{
		UseBlackboard(BlackboardAsset);
		RunBehaviorTree(BehaviorTreeAsset);
	}

	// Initialize tactical behavior
	if (TacticalBehaviorComponent)
	{
		TacticalBehaviorComponent->Initialize(this);
	}

	// Initialize pack coordination
	if (PackCoordinationComponent)
	{
		PackCoordinationComponent->Initialize(this);
	}
}

void ACombatAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	StateTimer += DeltaTime;
	UpdateAIState(DeltaTime);
	UpdateCombatBehavior(DeltaTime);
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		// Configure the possessed pawn for combat
		UE_LOG(LogTemp, Log, TEXT("CombatAIController possessed pawn: %s"), *InPawn->GetName());
	}
}

void ACombatAIController::SetupPerception()
{
	if (!AIPerceptionComponent)
		return;

	ConfigureSightSense();
	ConfigureHearingSense();
}

void ACombatAIController::ConfigureSightSense()
{
	UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = SightRadius;
		SightConfig->LoseSightRadius = LoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
		SightConfig->SetMaxAge(5.0f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;

		// Detect all pawns
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;

		AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
		AIPerceptionComponent->ConfigureSense(*SightConfig);
	}
}

void ACombatAIController::ConfigureHearingSense()
{
	UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = HearingRange;
		HearingConfig->SetMaxAge(3.0f);

		// Detect all sounds
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

		AIPerceptionComponent->ConfigureSense(*HearingConfig);
	}
}

void ACombatAIController::SetAIState(ECombat_AIState NewState)
{
	if (CurrentAIState != NewState)
	{
		TransitionToState(NewState);
	}
}

void ACombatAIController::TransitionToState(ECombat_AIState NewState)
{
	ECombat_AIState PreviousState = CurrentAIState;
	CurrentAIState = NewState;
	StateTimer = 0.0f;

	// Update blackboard
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(NewState));
	}

	// Log state transition
	UE_LOG(LogTemp, Log, TEXT("AI State Transition: %s -> %s"), 
		*UEnum::GetValueAsString(PreviousState), 
		*UEnum::GetValueAsString(NewState));

	// Handle state-specific logic
	switch (NewState)
	{
		case ECombat_AIState::Hunting:
			if (TacticalBehaviorComponent)
			{
				TacticalBehaviorComponent->StartHunting();
			}
			break;

		case ECombat_AIState::Combat:
			if (TacticalBehaviorComponent)
			{
				TacticalBehaviorComponent->StartCombat();
			}
			break;

		case ECombat_AIState::Fleeing:
			if (TacticalBehaviorComponent)
			{
				TacticalBehaviorComponent->StartFleeing();
			}
			break;

		case ECombat_AIState::Coordinating:
			if (PackCoordinationComponent)
			{
				PackCoordinationComponent->StartCoordination();
			}
			break;
	}
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
	AActor* PreviousTarget = CurrentTarget;
	CurrentTarget = NewTarget;
	bHasValidTarget = (NewTarget != nullptr);
	LastTargetUpdateTime = GetWorld()->GetTimeSeconds();

	// Update blackboard
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
		if (NewTarget)
		{
			BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), NewTarget->GetActorLocation());
		}
	}

	// Notify components
	if (TacticalBehaviorComponent)
	{
		TacticalBehaviorComponent->OnTargetChanged(PreviousTarget, NewTarget);
	}

	if (PackCoordinationComponent)
	{
		PackCoordinationComponent->OnTargetChanged(PreviousTarget, NewTarget);
	}
}

void ACombatAIController::UpdateAIState(float DeltaTime)
{
	// State-specific updates
	switch (CurrentAIState)
	{
		case ECombat_AIState::Idle:
			if (CurrentTarget)
			{
				SetAIState(ECombat_AIState::Hunting);
			}
			break;

		case ECombat_AIState::Hunting:
			if (!CurrentTarget)
			{
				SetAIState(ECombat_AIState::Idle);
			}
			else if (GetPawn() && FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation()) < 300.0f)
			{
				SetAIState(ECombat_AIState::Combat);
			}
			break;

		case ECombat_AIState::Combat:
			if (!CurrentTarget)
			{
				SetAIState(ECombat_AIState::Idle);
			}
			else if (GetPawn() && FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation()) > 800.0f)
			{
				SetAIState(ECombat_AIState::Hunting);
			}
			break;

		case ECombat_AIState::Fleeing:
			if (StateTimer > 5.0f) // Flee for 5 seconds
			{
				SetAIState(ECombat_AIState::Idle);
			}
			break;
	}
}

void ACombatAIController::UpdateCombatBehavior(float DeltaTime)
{
	EvaluateThreats();
	CoordinateWithPack();

	// Update components
	if (TacticalBehaviorComponent)
	{
		TacticalBehaviorComponent->UpdateBehavior(DeltaTime);
	}

	if (PackCoordinationComponent)
	{
		PackCoordinationComponent->UpdateCoordination(DeltaTime);
	}
}

void ACombatAIController::EvaluateThreats()
{
	if (!GetPawn())
		return;

	// Simple threat evaluation - can be expanded
	if (CurrentTarget)
	{
		float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
		
		// If target is too close and we're cautious, consider fleeing
		if (DistanceToTarget < 200.0f && CautiousLevel > 0.6f && AggressionLevel < 0.4f)
		{
			if (FMath::RandRange(0.0f, 1.0f) < CautiousLevel)
			{
				SetAIState(ECombat_AIState::Fleeing);
			}
		}
	}
}

void ACombatAIController::CoordinateWithPack()
{
	if (PackCoordinationComponent)
	{
		PackCoordinationComponent->UpdatePackBehavior();
	}
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	for (AActor* Actor : UpdatedActors)
	{
		if (Actor && Actor->IsA<APawn>() && Actor != GetPawn())
		{
			// Check if this is a potential target
			if (ShouldEngageTarget(Actor))
			{
				SetTarget(Actor);
				break;
			}
		}
	}
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Actor == CurrentTarget)
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			LastKnownTargetLocation = Stimulus.StimulusLocation;
			if (BlackboardComponent)
			{
				BlackboardComponent->SetValueAsVector(TEXT("LastKnownTargetLocation"), LastKnownTargetLocation);
			}
		}
	}
}

bool ACombatAIController::ShouldEngageTarget(AActor* PotentialTarget) const
{
	if (!PotentialTarget || !GetPawn())
		return false;

	// Don't engage pack members
	for (ACombatAIController* PackMember : PackMembers)
	{
		if (PackMember && PackMember->GetPawn() == PotentialTarget)
			return false;
	}

	// Don't engage pack leader
	if (PackLeader && PackLeader->GetPawn() == PotentialTarget)
		return false;

	// Check if it's a player character or other valid target
	if (PotentialTarget->IsA<APawn>())
	{
		float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), PotentialTarget->GetActorLocation());
		
		// Engage based on aggression level and distance
		float EngageChance = AggressionLevel * (1.0f - (DistanceToTarget / SightRadius));
		return FMath::RandRange(0.0f, 1.0f) < EngageChance;
	}

	return false;
}

void ACombatAIController::JoinPack(ACombatAIController* Leader)
{
	if (Leader && Leader != this)
	{
		LeavePack(); // Leave current pack first
		
		PackLeader = Leader;
		Leader->AddPackMember(this);
		
		if (PackCoordinationComponent)
		{
			PackCoordinationComponent->OnJoinedPack(Leader);
		}
	}
}

void ACombatAIController::LeavePack()
{
	if (PackLeader)
	{
		PackLeader->RemovePackMember(this);
		PackLeader = nullptr;
	}

	// If this was a leader, disband the pack
	for (ACombatAIController* Member : PackMembers)
	{
		if (Member)
		{
			Member->PackLeader = nullptr;
		}
	}
	PackMembers.Empty();

	if (PackCoordinationComponent)
	{
		PackCoordinationComponent->OnLeftPack();
	}
}

bool ACombatAIController::IsPackLeader() const
{
	return PackMembers.Num() > 0;
}

void ACombatAIController::AddPackMember(ACombatAIController* Member)
{
	if (Member && !PackMembers.Contains(Member))
	{
		PackMembers.Add(Member);
	}
}

void ACombatAIController::RemovePackMember(ACombatAIController* Member)
{
	PackMembers.Remove(Member);
}

FVector ACombatAIController::GetTacticalPosition(AActor* Target, ECombat_TacticalRole Role) const
{
	if (!Target || !GetPawn())
		return GetPawn()->GetActorLocation();

	FVector TargetLocation = Target->GetActorLocation();
	FVector CurrentLocation = GetPawn()->GetActorLocation();
	
	// Calculate tactical position based on role
	switch (Role)
	{
		case ECombat_TacticalRole::Flanker:
		{
			// Position to the side of the target
			FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
			FVector RightVector = FVector::CrossProduct(ToTarget, FVector::UpVector);
			return TargetLocation + RightVector * 400.0f;
		}
		
		case ECombat_TacticalRole::Ambusher:
		{
			// Position behind the target
			FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
			return TargetLocation - ToTarget * 300.0f;
		}
		
		case ECombat_TacticalRole::Distractor:
		{
			// Position in front of the target
			FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
			return TargetLocation + ToTarget * 500.0f;
		}
		
		default:
			return TargetLocation;
	}
}

bool ACombatAIController::CanExecuteTactic(ECombat_TacticType Tactic) const
{
	// Check if we have the resources/conditions to execute a tactic
	switch (Tactic)
	{
		case ECombat_TacticType::PackHunt:
			return PackMembers.Num() >= 2;
			
		case ECombat_TacticType::FlankingManeuver:
			return PackMembers.Num() >= 1;
			
		case ECombat_TacticType::Ambush:
			return CurrentAIState == ECombat_AIState::Hunting;
			
		case ECombat_TacticType::DirectAssault:
			return AggressionLevel > 0.6f;
			
		case ECombat_TacticType::HitAndRun:
			return true; // Always available
			
		default:
			return false;
	}
}