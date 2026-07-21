#include "NPC_DinosaurAdvancedAI.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurAdvancedAI::UNPC_DinosaurAdvancedAI()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	CurrentAIState = ENPC_DinosaurAIState::Idle;
	ThreatDetectionRadius = 3000.0f;
	HuntingRadius = 2500.0f;
	FleeRadius = 1500.0f;
	LearningRate = 0.1f;
	ExperiencePoints = 0;
	LastThreatEvaluation = 0.0f;
	ThreatEvaluationInterval = 2.0f;
	SafeZoneLocation = FVector::ZeroVector;
}

void UNPC_DinosaurAdvancedAI::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize learned behaviors with default values
	LearnedBehaviors.Add(TEXT("AggressionLevel"), 0.5f);
	LearnedBehaviors.Add(TEXT("CautionLevel"), 0.5f);
	LearnedBehaviors.Add(TEXT("HuntingEfficiency"), 0.3f);
	LearnedBehaviors.Add(TEXT("TerritorialDefense"), 0.4f);
	LearnedBehaviors.Add(TEXT("SocialCooperation"), 0.2f);
	
	// Set initial safe zone near spawn location
	SafeZoneLocation = GetOwner()->GetActorLocation();
}

void UNPC_DinosaurAdvancedAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!GetOwner())
		return;
	
	// Update environmental awareness periodically
	LastThreatEvaluation += DeltaTime;
	if (LastThreatEvaluation >= ThreatEvaluationInterval)
	{
		UpdateEnvironmentalAwareness();
		LastThreatEvaluation = 0.0f;
	}
	
	// Process learning and adaptation
	ProcessLearning();
	
	// Execute current behavior
	ExecuteAdvancedBehavior();
}

void UNPC_DinosaurAdvancedAI::EvaluateThreatLevel(AActor* ThreatActor)
{
	if (!ThreatActor || !GetOwner())
		return;
	
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
	float ThreatLevel = 0.0f;
	
	// Calculate threat based on distance and actor type
	if (Distance < FleeRadius)
	{
		ThreatLevel = 1.0f - (Distance / FleeRadius);
		
		// Check if this is a known threat
		if (!KnownThreats.Contains(ThreatActor))
		{
			KnownThreats.Add(ThreatActor);
		}
		
		// Increase caution level based on threat encounters
		float* CautionLevel = LearnedBehaviors.Find(TEXT("CautionLevel"));
		if (CautionLevel)
		{
			*CautionLevel = FMath::Clamp(*CautionLevel + (LearningRate * ThreatLevel), 0.0f, 1.0f);
		}
		
		// Change state based on threat level
		if (ThreatLevel > 0.7f && ShouldChangeState(ENPC_DinosaurAIState::Fleeing))
		{
			CurrentAIState = ENPC_DinosaurAIState::Fleeing;
		}
		else if (ThreatLevel > 0.4f && ShouldChangeState(ENPC_DinosaurAIState::Alert))
		{
			CurrentAIState = ENPC_DinosaurAIState::Alert;
		}
	}
}

void UNPC_DinosaurAdvancedAI::UpdateBehaviorBasedOnExperience()
{
	// Adjust behaviors based on accumulated experience
	if (ExperiencePoints > 100)
	{
		float* HuntingEfficiency = LearnedBehaviors.Find(TEXT("HuntingEfficiency"));
		if (HuntingEfficiency)
		{
			*HuntingEfficiency = FMath::Clamp(*HuntingEfficiency + 0.05f, 0.0f, 1.0f);
		}
	}
	
	if (ExperiencePoints > 200)
	{
		float* TerritorialDefense = LearnedBehaviors.Find(TEXT("TerritorialDefense"));
		if (TerritorialDefense)
		{
			*TerritorialDefense = FMath::Clamp(*TerritorialDefense + 0.03f, 0.0f, 1.0f);
		}
	}
	
	// Reset experience points after major learning milestone
	if (ExperiencePoints > 500)
	{
		ExperiencePoints = 250; // Keep some experience
	}
}

void UNPC_DinosaurAdvancedAI::LearnFromInteraction(AActor* InteractionTarget, bool bPositiveOutcome)
{
	if (!InteractionTarget)
		return;
	
	ExperiencePoints += bPositiveOutcome ? 10 : 5;
	
	if (bPositiveOutcome)
	{
		// Positive interaction - increase cooperation or reduce aggression
		float* SocialCooperation = LearnedBehaviors.Find(TEXT("SocialCooperation"));
		if (SocialCooperation)
		{
			*SocialCooperation = FMath::Clamp(*SocialCooperation + LearningRate, 0.0f, 1.0f);
		}
		
		// If this was food, remember the location
		if (InteractionTarget->GetName().Contains(TEXT("Food")) || InteractionTarget->GetName().Contains(TEXT("Prey")))
		{
			if (!KnownFood.Contains(InteractionTarget))
			{
				KnownFood.Add(InteractionTarget);
			}
		}
	}
	else
	{
		// Negative interaction - increase aggression or caution
		float* AggressionLevel = LearnedBehaviors.Find(TEXT("AggressionLevel"));
		if (AggressionLevel)
		{
			*AggressionLevel = FMath::Clamp(*AggressionLevel + (LearningRate * 0.5f), 0.0f, 1.0f);
		}
	}
}

FVector UNPC_DinosaurAdvancedAI::CalculateOptimalPosition()
{
	if (!GetOwner())
		return FVector::ZeroVector;
	
	FVector CurrentLocation = GetOwner()->GetActorLocation();
	FVector OptimalPosition = CurrentLocation;
	
	// Consider threats and move away from them
	for (AActor* Threat : KnownThreats)
	{
		if (Threat && IsValid(Threat))
		{
			FVector ThreatDirection = (CurrentLocation - Threat->GetActorLocation()).GetSafeNormal();
			float ThreatDistance = FVector::Dist(CurrentLocation, Threat->GetActorLocation());
			
			if (ThreatDistance < ThreatDetectionRadius)
			{
				float ThreatInfluence = 1.0f - (ThreatDistance / ThreatDetectionRadius);
				OptimalPosition += ThreatDirection * ThreatInfluence * 1000.0f;
			}
		}
	}
	
	// Consider food sources and move towards them if safe
	if (CurrentAIState != ENPC_DinosaurAIState::Fleeing)
	{
		for (AActor* Food : KnownFood)
		{
			if (Food && IsValid(Food))
			{
				FVector FoodDirection = (Food->GetActorLocation() - CurrentLocation).GetSafeNormal();
				float FoodDistance = FVector::Dist(CurrentLocation, Food->GetActorLocation());
				
				if (FoodDistance < HuntingRadius)
				{
					float FoodAttraction = 1.0f - (FoodDistance / HuntingRadius);
					OptimalPosition += FoodDirection * FoodAttraction * 500.0f;
				}
			}
		}
	}
	
	// Stay within reasonable bounds of safe zone
	float SafeZoneDistance = FVector::Dist(CurrentLocation, SafeZoneLocation);
	if (SafeZoneDistance > 5000.0f)
	{
		FVector SafeDirection = (SafeZoneLocation - CurrentLocation).GetSafeNormal();
		OptimalPosition += SafeDirection * 800.0f;
	}
	
	return OptimalPosition;
}

void UNPC_DinosaurAdvancedAI::ExecuteAdvancedBehavior()
{
	if (!GetOwner())
		return;
	
	switch (CurrentAIState)
	{
		case ENPC_DinosaurAIState::Idle:
		{
			// Random patrol behavior with learned caution
			float* CautionLevel = LearnedBehaviors.Find(TEXT("CautionLevel"));
			if (CautionLevel && *CautionLevel > 0.6f)
			{
				// High caution - stay near safe zone
				FVector SafeDirection = (SafeZoneLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
				// Move towards safe zone slowly
			}
			break;
		}
		
		case ENPC_DinosaurAIState::Hunting:
		{
			FVector OptimalPosition = CalculateOptimalPosition();
			float* HuntingEfficiency = LearnedBehaviors.Find(TEXT("HuntingEfficiency"));
			
			if (HuntingEfficiency && *HuntingEfficiency > 0.5f)
			{
				// Experienced hunter - use advanced tactics
				// Implement flanking or ambush behavior
			}
			break;
		}
		
		case ENPC_DinosaurAIState::Fleeing:
		{
			FVector FleePosition = CalculateOptimalPosition();
			// Move towards calculated safe position
			break;
		}
		
		case ENPC_DinosaurAIState::Alert:
		{
			// Scan environment for threats
			UpdateEnvironmentalAwareness();
			break;
		}
		
		default:
			break;
	}
}

void UNPC_DinosaurAdvancedAI::UpdateEnvironmentalAwareness()
{
	if (!GetOwner())
		return;
	
	// Clean up invalid actors
	KnownThreats.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
	KnownFood.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
	
	// Scan for new threats and food sources
	TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
	
	for (AActor* Actor : NearbyActors)
	{
		if (!Actor || Actor == GetOwner())
			continue;
		
		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
		
		if (Distance <= ThreatDetectionRadius)
		{
			// Check if this could be a threat or food
			FString ActorName = Actor->GetName().ToLower();
			
			if (ActorName.Contains(TEXT("player")) || ActorName.Contains(TEXT("predator")))
			{
				EvaluateThreatLevel(Actor);
			}
			else if (ActorName.Contains(TEXT("prey")) || ActorName.Contains(TEXT("food")))
			{
				if (!KnownFood.Contains(Actor))
				{
					KnownFood.Add(Actor);
				}
			}
		}
	}
}

void UNPC_DinosaurAdvancedAI::ProcessLearning()
{
	// Gradually decay extreme behaviors towards balanced values
	for (auto& Behavior : LearnedBehaviors)
	{
		float& Value = Behavior.Value;
		float TargetValue = 0.5f; // Balanced baseline
		float DecayRate = 0.001f; // Very slow decay
		
		if (Value > TargetValue)
		{
			Value = FMath::Max(Value - DecayRate, TargetValue);
		}
		else if (Value < TargetValue)
		{
			Value = FMath::Min(Value + DecayRate, TargetValue);
		}
	}
}

bool UNPC_DinosaurAdvancedAI::ShouldChangeState(ENPC_DinosaurAIState NewState)
{
	// Implement state transition logic based on learned behaviors
	float* CautionLevel = LearnedBehaviors.Find(TEXT("CautionLevel"));
	float* AggressionLevel = LearnedBehaviors.Find(TEXT("AggressionLevel"));
	
	switch (NewState)
	{
		case ENPC_DinosaurAIState::Fleeing:
			return CautionLevel ? *CautionLevel > 0.4f : true;
			
		case ENPC_DinosaurAIState::Hunting:
			return AggressionLevel ? *AggressionLevel > 0.6f : false;
			
		case ENPC_DinosaurAIState::Alert:
			return true; // Always allow alert state
			
		default:
			return true;
	}
}