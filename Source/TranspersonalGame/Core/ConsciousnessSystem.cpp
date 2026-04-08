#include "ConsciousnessSystem.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UConsciousnessSystem::UConsciousnessSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	BaseAwareness = 0.5f;
	TransitionSpeed = 1.0f;
	bIsTransitioning = false;
	
	// Set default consciousness level
	CurrentLevel.State = EConsciousnessState::Ordinary;
	CurrentLevel.Depth = 0.0f;
	CurrentLevel.Clarity = 1.0f;
	CurrentLevel.Integration = 0.0f;
}

void UConsciousnessSystem::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize target level to current
	TargetLevel = CurrentLevel;
	
	UE_LOG(LogTemp, Warning, TEXT("Consciousness System Initialized"));
}

void UConsciousnessSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsTransitioning)
	{
		UpdateTransition(DeltaTime);
	}
	
	CalculateClarity();
}

void UConsciousnessSystem::TransitionToState(EConsciousnessState NewState, float TargetDepth)
{
	if (CurrentLevel.State == NewState && FMath::IsNearlyEqual(CurrentLevel.Depth, TargetDepth, 0.1f))
	{
		return; // Already at target state
	}
	
	TargetLevel.State = NewState;
	TargetLevel.Depth = FMath::Clamp(TargetDepth, 0.0f, 1.0f);
	
	// Set state-specific properties
	switch (NewState)
	{
		case EConsciousnessState::Ordinary:
			TargetLevel.Clarity = 1.0f;
			break;
		case EConsciousnessState::Dreaming:
			TargetLevel.Clarity = 0.3f;
			break;
		case EConsciousnessState::Meditation:
			TargetLevel.Clarity = 0.8f;
			break;
		case EConsciousnessState::Transcendent:
			TargetLevel.Clarity = 0.9f;
			break;
		case EConsciousnessState::Unity:
			TargetLevel.Clarity = 1.0f;
			break;
		case EConsciousnessState::Shadow:
			TargetLevel.Clarity = 0.2f;
			break;
	}
	
	bIsTransitioning = true;
	
	UE_LOG(LogTemp, Warning, TEXT("Transitioning to consciousness state: %d"), (int32)NewState);
}

void UConsciousnessSystem::ModifyAwareness(float Delta)
{
	float NewAwareness = FMath::Clamp(BaseAwareness + Delta, 0.0f, 1.0f);
	
	if (!FMath::IsNearlyEqual(BaseAwareness, NewAwareness, 0.01f))
	{
		BaseAwareness = NewAwareness;
		OnAwarenessShift.Broadcast(BaseAwareness);
		
		UE_LOG(LogTemp, Log, TEXT("Awareness modified to: %f"), BaseAwareness);
	}
}

void UConsciousnessSystem::IntegrateExperience(float IntegrationAmount)
{
	CurrentLevel.Integration = FMath::Clamp(CurrentLevel.Integration + IntegrationAmount, 0.0f, 1.0f);
	
	UE_LOG(LogTemp, Log, TEXT("Experience integrated. Integration level: %f"), CurrentLevel.Integration);
}

bool UConsciousnessSystem::IsInTranscendentState() const
{
	return CurrentLevel.State == EConsciousnessState::Transcendent || 
		   CurrentLevel.State == EConsciousnessState::Unity;
}

void UConsciousnessSystem::UpdateTransition(float DeltaTime)
{
	float TransitionRate = TransitionSpeed * DeltaTime;
	bool bTransitionComplete = true;
	
	// Interpolate depth
	if (!FMath::IsNearlyEqual(CurrentLevel.Depth, TargetLevel.Depth, 0.01f))
	{
		CurrentLevel.Depth = FMath::FInterpTo(CurrentLevel.Depth, TargetLevel.Depth, DeltaTime, TransitionSpeed);
		bTransitionComplete = false;
	}
	
	// Interpolate clarity
	if (!FMath::IsNearlyEqual(CurrentLevel.Clarity, TargetLevel.Clarity, 0.01f))
	{
		CurrentLevel.Clarity = FMath::FInterpTo(CurrentLevel.Clarity, TargetLevel.Clarity, DeltaTime, TransitionSpeed * 0.5f);
		bTransitionComplete = false;
	}
	
	// Update state when depth reaches threshold
	if (CurrentLevel.State != TargetLevel.State && CurrentLevel.Depth > 0.5f)
	{
		EConsciousnessState OldState = CurrentLevel.State;
		CurrentLevel.State = TargetLevel.State;
		
		OnConsciousnessChanged.Broadcast(CurrentLevel.State, CurrentLevel.Depth);
		
		UE_LOG(LogTemp, Warning, TEXT("Consciousness state changed from %d to %d"), (int32)OldState, (int32)CurrentLevel.State);
	}
	
	// Check if transition is complete
	if (bTransitionComplete && CurrentLevel.State == TargetLevel.State)
	{
		bIsTransitioning = false;
		UE_LOG(LogTemp, Log, TEXT("Consciousness transition completed"));
	}
}

void UConsciousnessSystem::CalculateClarity()
{
	// Clarity is affected by awareness and current state
	float AwarenessModifier = BaseAwareness * 0.5f + 0.5f; // Scale 0.5-1.0
	
	// Apply state-specific clarity modulation
	float StateClarity = CurrentLevel.Clarity;
	
	// Integration affects clarity positively
	float IntegrationBonus = CurrentLevel.Integration * 0.2f;
	
	float FinalClarity = FMath::Clamp(StateClarity * AwarenessModifier + IntegrationBonus, 0.0f, 1.0f);
	
	if (!FMath::IsNearlyEqual(CurrentLevel.Clarity, FinalClarity, 0.01f))
	{
		CurrentLevel.Clarity = FinalClarity;
	}
}