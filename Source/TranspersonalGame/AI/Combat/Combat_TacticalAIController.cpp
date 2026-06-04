#include "Combat_TacticalAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ACombat_TacticalAIController::ACombat_TacticalAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize AI Perception
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	// Setup Sight Configuration
	SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1600.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->SetMaxAge(5.0f);
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 900.0f;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;

	// Setup Hearing Configuration
	HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 1200.0f;
	HearingConfig->SetMaxAge(3.0f);
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

	// Configure Perception Component
	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->ConfigureSense(*HearingConfig);
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	// Initialize Combat Parameters
	AttackRange = 300.0f;
	DetectionRange = 1500.0f;
	Aggression = 75.0f;
	Intelligence = 60.0f;

	// Initialize Pack Behavior
	bIsPackLeader = false;
	PackCohesion = 80.0f;
	PackLeader = nullptr;

	// Initialize Tactical State
	CurrentTacticalState = ECombat_TacticalState::Patrol;
	CurrentFormation.FormationRadius = 500.0f;
	CurrentFormation.bMaintainFormation = true;
}

void ACombat_TacticalAIController::BeginPlay()
{
	Super::BeginPlay();

	// Bind perception events
	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_TacticalAIController::OnPerceptionUpdated);
	}

	// Initialize blackboard values
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(CurrentTacticalState));
		GetBlackboardComponent()->SetValueAsFloat(TEXT("AttackRange"), AttackRange);
		GetBlackboardComponent()->SetValueAsFloat(TEXT("DetectionRange"), DetectionRange);
		GetBlackboardComponent()->SetValueAsFloat(TEXT("Aggression"), Aggression);
	}
}

void ACombat_TacticalAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTacticalBehavior(DeltaTime);
	ProcessPackCommunication();
}

void ACombat_TacticalAIController::SetTacticalState(ECombat_TacticalState NewState)
{
	if (CurrentTacticalState != NewState)
	{
		CurrentTacticalState = NewState;
		
		if (GetBlackboardComponent())
		{
			GetBlackboardComponent()->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(CurrentTacticalState));
		}

		BroadcastTacticalState();
	}
}

void ACombat_TacticalAIController::InitiateAttack(AActor* Target)
{
	if (!Target) return;

	SetTacticalState(ECombat_TacticalState::Attack);
	
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), Target);
	}

	// Notify pack members of attack
	for (ACombat_TacticalAIController* PackMember : PackMembers)
	{
		if (PackMember && PackMember != this)
		{
			PackMember->SetTacticalState(ECombat_TacticalState::Attack);
			if (PackMember->GetBlackboardComponent())
			{
				PackMember->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), Target);
			}
		}
	}
}

void ACombat_TacticalAIController::ExecuteFlankingManeuver(AActor* Target)
{
	if (!Target) return;

	FVector FlankPosition = CalculateFlankingPosition(Target);
	SetTacticalState(ECombat_TacticalState::Flank);

	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsVector(TEXT("FlankPosition"), FlankPosition);
		GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), Target);
	}
}

void ACombat_TacticalAIController::SetupAmbush(FVector AmbushLocation)
{
	SetTacticalState(ECombat_TacticalState::Ambush);

	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsVector(TEXT("AmbushLocation"), AmbushLocation);
	}

	// Position pack members for ambush
	for (int32 i = 0; i < PackMembers.Num(); i++)
	{
		if (PackMembers[i] && PackMembers[i] != this)
		{
			FVector MemberAmbushPos = AmbushLocation + FVector(
				FMath::Cos(i * 2.0f * PI / PackMembers.Num()) * 400.0f,
				FMath::Sin(i * 2.0f * PI / PackMembers.Num()) * 400.0f,
				0.0f
			);
			
			PackMembers[i]->SetupAmbush(MemberAmbushPos);
		}
	}
}

void ACombat_TacticalAIController::JoinPack(ACombat_TacticalAIController* Leader)
{
	if (!Leader) return;

	PackLeader = Leader;
	bIsPackLeader = false;

	if (Leader != this)
	{
		Leader->PackMembers.AddUnique(this);
	}
}

void ACombat_TacticalAIController::FormPack(const TArray<ACombat_TacticalAIController*>& Members)
{
	PackMembers = Members;
	bIsPackLeader = true;

	for (ACombat_TacticalAIController* Member : Members)
	{
		if (Member && Member != this)
		{
			Member->JoinPack(this);
		}
	}
}

void ACombat_TacticalAIController::UpdateFormation(const FCombat_TacticalFormation& NewFormation)
{
	CurrentFormation = NewFormation;

	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsVector(TEXT("FormationCenter"), NewFormation.LeaderPosition);
		GetBlackboardComponent()->SetValueAsFloat(TEXT("FormationRadius"), NewFormation.FormationRadius);
	}
}

void ACombat_TacticalAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	for (AActor* Actor : UpdatedActors)
	{
		if (!Actor) continue;

		// Check if this is a player character
		if (Actor->IsA<ACharacter>())
		{
			FActorPerceptionBlueprintInfo PerceptionInfo;
			AIPerceptionComponent->GetActorsPerception(Actor, PerceptionInfo);

			for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
			{
				if (Stimulus.WasSuccessfullySensed())
				{
					// Player detected - evaluate threat
					float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
					
					if (DistanceToPlayer <= AttackRange && CurrentTacticalState == ECombat_TacticalState::Patrol)
					{
						InitiateAttack(Actor);
					}
					else if (DistanceToPlayer <= DetectionRange && CurrentTacticalState == ECombat_TacticalState::Patrol)
					{
						SetTacticalState(ECombat_TacticalState::Hunt);
						if (GetBlackboardComponent())
						{
							GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), Actor);
						}
					}
				}
			}
		}
	}
}

void ACombat_TacticalAIController::UpdateTacticalBehavior(float DeltaTime)
{
	if (!GetPawn()) return;

	switch (CurrentTacticalState)
	{
		case ECombat_TacticalState::Patrol:
			// Patrol behavior - look for threats
			break;

		case ECombat_TacticalState::Hunt:
			// Hunt behavior - pursue target
			if (GetBlackboardComponent())
			{
				AActor* Target = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
				if (Target)
				{
					float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
					if (Distance <= AttackRange)
					{
						InitiateAttack(Target);
					}
				}
			}
			break;

		case ECombat_TacticalState::Attack:
			// Attack behavior - coordinate with pack
			break;

		case ECombat_TacticalState::Flank:
			// Flanking behavior - move to flanking position
			break;

		case ECombat_TacticalState::Ambush:
			// Ambush behavior - wait for target
			break;

		case ECombat_TacticalState::Retreat:
			// Retreat behavior - fall back and regroup
			break;
	}
}

void ACombat_TacticalAIController::ProcessPackCommunication()
{
	if (!bIsPackLeader) return;

	// Update pack formation
	if (CurrentFormation.bMaintainFormation && PackMembers.Num() > 0)
	{
		FVector LeaderLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
		CurrentFormation.LeaderPosition = LeaderLocation;

		for (int32 i = 0; i < PackMembers.Num(); i++)
		{
			if (PackMembers[i] && PackMembers[i] != this)
			{
				FVector FormationPos = LeaderLocation + FVector(
					FMath::Cos(i * 2.0f * PI / PackMembers.Num()) * CurrentFormation.FormationRadius,
					FMath::Sin(i * 2.0f * PI / PackMembers.Num()) * CurrentFormation.FormationRadius,
					0.0f
				);

				if (PackMembers[i]->GetBlackboardComponent())
				{
					PackMembers[i]->GetBlackboardComponent()->SetValueAsVector(TEXT("FormationPosition"), FormationPos);
				}
			}
		}
	}
}

FVector ACombat_TacticalAIController::CalculateFlankingPosition(AActor* Target)
{
	if (!Target || !GetPawn()) return FVector::ZeroVector;

	FVector TargetLocation = Target->GetActorLocation();
	FVector MyLocation = GetPawn()->GetActorLocation();
	FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
	
	// Calculate flanking position 90 degrees to the right
	FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector).GetSafeNormal();
	FVector FlankPosition = TargetLocation + (FlankDirection * AttackRange * 1.5f);

	return FlankPosition;
}

bool ACombat_TacticalAIController::IsInAttackRange(AActor* Target) const
{
	if (!Target || !GetPawn()) return false;

	float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
	return Distance <= AttackRange;
}

void ACombat_TacticalAIController::BroadcastTacticalState()
{
	// Notify pack members of state change
	for (ACombat_TacticalAIController* PackMember : PackMembers)
	{
		if (PackMember && PackMember != this)
		{
			// Pack members can react to leader's state changes
			if (bIsPackLeader && CurrentTacticalState == ECombat_TacticalState::Attack)
			{
				PackMember->SetTacticalState(ECombat_TacticalState::Attack);
			}
		}
	}
}