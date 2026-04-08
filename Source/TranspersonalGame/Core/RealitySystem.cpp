#include "RealitySystem.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

URealitySystem::URealitySystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	DistortionIntensity = 0.0f;
	LayerTransitionSpeed = 2.0f;
	bIsTransitioning = false;
	DistortionTimer = 0.0f;
	DistortionDuration = 0.0f;
	BaseDistortionIntensity = 0.0f;
	
	// Set default reality parameters
	CurrentReality.Stability = 1.0f;
	CurrentReality.Coherence = 1.0f;
	CurrentReality.Permeability = 0.0f;
	CurrentReality.SymbolicDensity = 0.0f;
	CurrentReality.ActiveLayer = ERealityLayer::Physical;
}

void URealitySystem::BeginPlay()
{
	Super::BeginPlay();
	
	// Find consciousness system reference
	ConsciousnessRef = GetOwner()->FindComponentByClass<UConsciousnessSystem>();
	
	// Initialize target reality to current
	TargetReality = CurrentReality;
	
	UE_LOG(LogTemp, Warning, TEXT("Reality System Initialized"));
}

void URealitySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Sync with consciousness system
	SyncWithConsciousness();
	
	// Update transitions
	if (bIsTransitioning)
	{
		UpdateRealityTransition(DeltaTime);
	}
	
	// Update distortion effects
	if (DistortionTimer > 0.0f)
	{
		UpdateDistortionEffects(DeltaTime);
	}
	
	// Calculate dynamic reality parameters
	CalculateRealityParameters();
}

void URealitySystem::ShiftToLayer(ERealityLayer NewLayer)
{
	if (CurrentReality.ActiveLayer == NewLayer)
	{
		return; // Already at target layer
	}
	
	TargetReality.ActiveLayer = NewLayer;
	
	// Set layer-specific parameters
	switch (NewLayer)
	{
		case ERealityLayer::Physical:
			TargetReality.Stability = 1.0f;
			TargetReality.Coherence = 1.0f;
			TargetReality.Permeability = 0.0f;
			TargetReality.SymbolicDensity = 0.1f;
			break;
		case ERealityLayer::Emotional:
			TargetReality.Stability = 0.7f;
			TargetReality.Coherence = 0.8f;
			TargetReality.Permeability = 0.3f;
			TargetReality.SymbolicDensity = 0.4f;
			break;
		case ERealityLayer::Mental:
			TargetReality.Stability = 0.5f;
			TargetReality.Coherence = 0.9f;
			TargetReality.Permeability = 0.6f;
			TargetReality.SymbolicDensity = 0.7f;
			break;
		case ERealityLayer::Archetypal:
			TargetReality.Stability = 0.3f;
			TargetReality.Coherence = 0.6f;
			TargetReality.Permeability = 0.8f;
			TargetReality.SymbolicDensity = 0.9f;
			break;
		case ERealityLayer::Causal:
			TargetReality.Stability = 0.1f;
			TargetReality.Coherence = 0.4f;
			TargetReality.Permeability = 1.0f;
			TargetReality.SymbolicDensity = 0.5f;
			break;
		case ERealityLayer::Void:
			TargetReality.Stability = 0.0f;
			TargetReality.Coherence = 0.0f;
			TargetReality.Permeability = 1.0f;
			TargetReality.SymbolicDensity = 0.0f;
			break;
	}
	
	bIsTransitioning = true;
	
	UE_LOG(LogTemp, Warning, TEXT("Shifting to reality layer: %d"), (int32)NewLayer);
}

void URealitySystem::ApplyRealityDistortion(float Intensity, float Duration)
{
	BaseDistortionIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	DistortionDuration = FMath::Max(Duration, 0.1f);
	DistortionTimer = DistortionDuration;
	
	OnRealityDistortion.Broadcast(BaseDistortionIntensity, DistortionDuration);
	
	UE_LOG(LogTemp, Log, TEXT("Reality distortion applied: Intensity=%f, Duration=%f"), Intensity, Duration);
}

void URealitySystem::ModifyStability(float Delta)
{
	float NewStability = FMath::Clamp(CurrentReality.Stability + Delta, 0.0f, 1.0f);
	CurrentReality.Stability = NewStability;
	
	UE_LOG(LogTemp, Log, TEXT("Reality stability modified to: %f"), CurrentReality.Stability);
}

void URealitySystem::SetPermeability(float NewPermeability)
{
	CurrentReality.Permeability = FMath::Clamp(NewPermeability, 0.0f, 1.0f);
	
	UE_LOG(LogTemp, Log, TEXT("Reality permeability set to: %f"), CurrentReality.Permeability);
}

void URealitySystem::UpdateRealityTransition(float DeltaTime)
{
	float TransitionRate = LayerTransitionSpeed * DeltaTime;
	bool bTransitionComplete = true;
	
	// Interpolate stability
	if (!FMath::IsNearlyEqual(CurrentReality.Stability, TargetReality.Stability, 0.01f))
	{
		CurrentReality.Stability = FMath::FInterpTo(CurrentReality.Stability, TargetReality.Stability, DeltaTime, LayerTransitionSpeed);
		bTransitionComplete = false;
	}
	
	// Interpolate coherence
	if (!FMath::IsNearlyEqual(CurrentReality.Coherence, TargetReality.Coherence, 0.01f))
	{
		CurrentReality.Coherence = FMath::FInterpTo(CurrentReality.Coherence, TargetReality.Coherence, DeltaTime, LayerTransitionSpeed);
		bTransitionComplete = false;
	}
	
	// Interpolate permeability
	if (!FMath::IsNearlyEqual(CurrentReality.Permeability, TargetReality.Permeability, 0.01f))
	{
		CurrentReality.Permeability = FMath::FInterpTo(CurrentReality.Permeability, TargetReality.Permeability, DeltaTime, LayerTransitionSpeed);
		bTransitionComplete = false;
	}
	
	// Interpolate symbolic density
	if (!FMath::IsNearlyEqual(CurrentReality.SymbolicDensity, TargetReality.SymbolicDensity, 0.01f))
	{
		CurrentReality.SymbolicDensity = FMath::FInterpTo(CurrentReality.SymbolicDensity, TargetReality.SymbolicDensity, DeltaTime, LayerTransitionSpeed);
		bTransitionComplete = false;
	}
	
	// Update layer when transition reaches threshold
	if (CurrentReality.ActiveLayer != TargetReality.ActiveLayer)
	{
		float TransitionProgress = 1.0f - FMath::Abs(CurrentReality.Stability - TargetReality.Stability);
		if (TransitionProgress > 0.7f)
		{
			ERealityLayer OldLayer = CurrentReality.ActiveLayer;
			CurrentReality.ActiveLayer = TargetReality.ActiveLayer;
			
			OnRealityShift.Broadcast(CurrentReality.ActiveLayer);
			
			UE_LOG(LogTemp, Warning, TEXT("Reality layer changed from %d to %d"), (int32)OldLayer, (int32)CurrentReality.ActiveLayer);
		}
		bTransitionComplete = false;
	}
	
	// Check if transition is complete
	if (bTransitionComplete)
	{
		bIsTransitioning = false;
		UE_LOG(LogTemp, Log, TEXT("Reality transition completed"));
	}
}

void URealitySystem::UpdateDistortionEffects(float DeltaTime)
{
	DistortionTimer -= DeltaTime;
	
	if (DistortionTimer <= 0.0f)
	{
		DistortionIntensity = 0.0f;
		DistortionTimer = 0.0f;
		UE_LOG(LogTemp, Log, TEXT("Reality distortion ended"));
	}
	else
	{
		// Calculate current distortion intensity with fade-out
		float NormalizedTime = DistortionTimer / DistortionDuration;
		DistortionIntensity = BaseDistortionIntensity * NormalizedTime;
	}
}

void URealitySystem::SyncWithConsciousness()
{
	if (!ConsciousnessRef)
	{
		return;
	}
	
	// Reality layer should match consciousness state
	EConsciousnessState CurrentConsciousnessState = ConsciousnessRef->CurrentLevel.State;
	ERealityLayer CorrespondingLayer = ERealityLayer::Physical;
	
	switch (CurrentConsciousnessState)
	{
		case EConsciousnessState::Ordinary:
			CorrespondingLayer = ERealityLayer::Physical;
			break;
		case EConsciousnessState::Dreaming:
			CorrespondingLayer = ERealityLayer::Emotional;
			break;
		case EConsciousnessState::Meditation:
			CorrespondingLayer = ERealityLayer::Mental;
			break;
		case EConsciousnessState::Transcendent:
			CorrespondingLayer = ERealityLayer::Archetypal;
			break;
		case EConsciousnessState::Unity:
			CorrespondingLayer = ERealityLayer::Causal;
			break;
		case EConsciousnessState::Shadow:
			CorrespondingLayer = ERealityLayer::Void;
			break;
	}
	
	// Auto-shift if consciousness depth is high enough
	if (ConsciousnessRef->GetConsciousnessDepth() > 0.6f && CurrentReality.ActiveLayer != CorrespondingLayer)
	{
		ShiftToLayer(CorrespondingLayer);
	}
}

void URealitySystem::CalculateRealityParameters()
{
	// Apply distortion effects to coherence
	float DistortedCoherence = CurrentReality.Coherence * (1.0f - DistortionIntensity * 0.5f);
	CurrentReality.Coherence = FMath::Clamp(DistortedCoherence, 0.0f, 1.0f);
	
	// Consciousness integration affects stability
	if (ConsciousnessRef)
	{
		float IntegrationBonus = ConsciousnessRef->CurrentLevel.Integration * 0.2f;
		CurrentReality.Stability = FMath::Clamp(CurrentReality.Stability + IntegrationBonus, 0.0f, 1.0f);
	}
}