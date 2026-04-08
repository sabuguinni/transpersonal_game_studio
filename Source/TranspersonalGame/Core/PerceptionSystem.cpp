#include "PerceptionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UPerceptionSystem::UPerceptionSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	CurrentMode = EPerceptionMode::Normal;
	PerceptionRange = 2000.0f;
	Sensitivity = 0.5f;
	PerceptionUpdateTimer = 0.0f;
	PerceptionUpdateInterval = 0.5f; // Update every half second
	
	// Initialize default normal perception filter
	FPerceptionFilter NormalFilter;
	NormalFilter.Mode = EPerceptionMode::Normal;
	NormalFilter.Intensity = 1.0f;
	NormalFilter.Range = PerceptionRange;
	NormalFilter.bActive = true;
	ActiveFilters.Add(NormalFilter);
}

void UPerceptionSystem::BeginPlay()
{
	Super::BeginPlay();
	
	// Find system references
	ConsciousnessRef = GetOwner()->FindComponentByClass<UConsciousnessSystem>();
	RealityRef = GetOwner()->FindComponentByClass<URealitySystem>();
	
	UE_LOG(LogTemp, Warning, TEXT("Perception System Initialized"));
}

void UPerceptionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Update perception mode based on consciousness state
	UpdatePerceptionMode();
	
	// Periodic environment scanning
	PerceptionUpdateTimer += DeltaTime;
	if (PerceptionUpdateTimer >= PerceptionUpdateInterval)
	{
		ScanEnvironment();
		ProcessPerceivedObjects();
		PerceptionUpdateTimer = 0.0f;
	}
}

void UPerceptionSystem::SetPerceptionMode(EPerceptionMode NewMode)
{
	if (CurrentMode == NewMode)
	{
		return;
	}
	
	EPerceptionMode OldMode = CurrentMode;
	CurrentMode = NewMode;
	
	// Update active filters
	for (FPerceptionFilter& Filter : ActiveFilters)
	{
		Filter.bActive = (Filter.Mode == CurrentMode);
	}
	
	OnPerceptionModeChanged.Broadcast(CurrentMode);
	
	UE_LOG(LogTemp, Warning, TEXT("Perception mode changed from %d to %d"), (int32)OldMode, (int32)CurrentMode);
}

void UPerceptionSystem::AddPerceptionFilter(EPerceptionMode Mode, float Intensity)
{
	// Check if filter already exists
	for (FPerceptionFilter& Filter : ActiveFilters)
	{
		if (Filter.Mode == Mode)
		{
			Filter.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
			Filter.bActive = true;
			UE_LOG(LogTemp, Log, TEXT("Updated perception filter %d with intensity %f"), (int32)Mode, Intensity);
			return;
		}
	}
	
	// Add new filter
	FPerceptionFilter NewFilter;
	NewFilter.Mode = Mode;
	NewFilter.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	NewFilter.Range = PerceptionRange;
	NewFilter.bActive = true;
	
	ActiveFilters.Add(NewFilter);
	
	UE_LOG(LogTemp, Log, TEXT("Added perception filter %d with intensity %f"), (int32)Mode, Intensity);
}

void UPerceptionSystem::RemovePerceptionFilter(EPerceptionMode Mode)
{
	for (int32 i = ActiveFilters.Num() - 1; i >= 0; i--)
	{
		if (ActiveFilters[i].Mode == Mode && Mode != EPerceptionMode::Normal)
		{
			ActiveFilters.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Removed perception filter %d"), (int32)Mode);
			break;
		}
	}
}

void UPerceptionSystem::ModifySensitivity(float Delta)
{
	float NewSensitivity = FMath::Clamp(Sensitivity + Delta, 0.0f, 1.0f);
	
	if (!FMath::IsNearlyEqual(Sensitivity, NewSensitivity, 0.01f))
	{
		Sensitivity = NewSensitivity;
		UE_LOG(LogTemp, Log, TEXT("Perception sensitivity modified to: %f"), Sensitivity);
	}
}

bool UPerceptionSystem::CanPerceiveAuras() const
{
	for (const FPerceptionFilter& Filter : ActiveFilters)
	{
		if (Filter.bActive && (Filter.Mode == EPerceptionMode::Aura || Filter.Mode == EPerceptionMode::Energetic))
		{
			return Filter.Intensity > 0.3f;
		}
	}
	return false;
}

bool UPerceptionSystem::CanPerceiveSymbols() const
{
	for (const FPerceptionFilter& Filter : ActiveFilters)
	{
		if (Filter.bActive && (Filter.Mode == EPerceptionMode::Symbolic || Filter.Mode == EPerceptionMode::Archetypal))
		{
			return Filter.Intensity > 0.3f;
		}
	}
	return false;
}

FPerceptualObject UPerceptionSystem::GetObjectPerception(AActor* Object) const
{
	for (const FPerceptualObject& PerceivedObj : PerceivedObjects)
	{
		if (PerceivedObj.Actor == Object)
		{
			return PerceivedObj;
		}
	}
	
	// Return default perception if not found
	FPerceptualObject DefaultPerception;
	DefaultPerception.Actor = Object;
	return DefaultPerception;
}

void UPerceptionSystem::ScanEnvironment()
{
	if (!GetWorld())
	{
		return;
	}
	
	// Clear previous perceptions
	PerceivedObjects.Empty();
	
	// Get owner location
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	
	// Sphere trace to find nearby objects
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		OwnerLocation,
		OwnerLocation,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeSphere(PerceptionRange),
		QueryParams
	);
	
	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			if (Hit.GetActor())
			{
				FPerceptualObject PerceptionData = AnalyzeObject(Hit.GetActor());
				ApplyPerceptionFilters(PerceptionData);
				PerceivedObjects.Add(PerceptionData);
			}
		}
	}
	
	UE_LOG(LogTemp, VeryVerbose, TEXT("Scanned environment: %d objects perceived"), PerceivedObjects.Num());
}

void UPerceptionSystem::UpdatePerceptionMode()
{
	if (!ConsciousnessRef || !RealityRef)
	{
		return;
	}
	
	// Determine perception mode based on consciousness state and reality layer
	EConsciousnessState ConsciousnessState = ConsciousnessRef->CurrentLevel.State;
	ERealityLayer RealityLayer = RealityRef->GetCurrentLayer();
	
	EPerceptionMode NewMode = EPerceptionMode::Normal;
	
	// Map consciousness states to perception modes
	switch (ConsciousnessState)
	{
		case EConsciousnessState::Ordinary:
			NewMode = EPerceptionMode::Normal;
			break;
		case EConsciousnessState::Dreaming:
			NewMode = EPerceptionMode::Symbolic;
			break;
		case EConsciousnessState::Meditation:
			NewMode = EPerceptionMode::Aura;
			break;
		case EConsciousnessState::Transcendent:
			NewMode = EPerceptionMode::Energetic;
			break;
		case EConsciousnessState::Unity:
			NewMode = EPerceptionMode::Unity;
			break;
		case EConsciousnessState::Shadow:
			NewMode = EPerceptionMode::Archetypal;
			break;
	}
	
	// Reality layer can override perception mode
	if (RealityLayer == ERealityLayer::Archetypal || RealityLayer == ERealityLayer::Causal)
	{
		NewMode = EPerceptionMode::Archetypal;
	}
	else if (RealityLayer == ERealityLayer::Void)
	{
		NewMode = EPerceptionMode::Unity;
	}
	
	// Auto-set perception mode if consciousness depth is sufficient
	if (ConsciousnessRef->GetConsciousnessDepth() > 0.5f && CurrentMode != NewMode)
	{
		SetPerceptionMode(NewMode);
	}
}

void UPerceptionSystem::ProcessPerceivedObjects()
{
	for (const FPerceptualObject& PerceivedObj : PerceivedObjects)
	{
		if (PerceivedObj.Actor)
		{
			OnObjectPerceived.Broadcast(PerceivedObj.Actor, PerceivedObj);
		}
	}
}

FPerceptualObject UPerceptionSystem::AnalyzeObject(AActor* Object)
{
	FPerceptualObject PerceptionData;
	PerceptionData.Actor = Object;
	
	if (!Object)
	{
		return PerceptionData;
	}
	
	// Calculate distance for energy level
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Object->GetActorLocation());
	float NormalizedDistance = FMath::Clamp(1.0f - (Distance / PerceptionRange), 0.0f, 1.0f);
	
	// Base energy level on distance and object properties
	PerceptionData.EnergyLevel = NormalizedDistance * 0.5f;
	
	// Determine aura color based on object type/name
	FString ObjectName = Object->GetName();
	if (ObjectName.Contains(TEXT("Enemy")) || ObjectName.Contains(TEXT("Shadow")))
	{
		PerceptionData.AuraColor = FVector(1.0f, 0.2f, 0.2f); // Red for hostile
		PerceptionData.EnergyLevel += 0.3f;
	}
	else if (ObjectName.Contains(TEXT("Pickup")) || ObjectName.Contains(TEXT("Item")))
	{
		PerceptionData.AuraColor = FVector(0.2f, 1.0f, 0.2f); // Green for beneficial
		PerceptionData.EnergyLevel += 0.2f;
	}
	else if (ObjectName.Contains(TEXT("Portal")) || ObjectName.Contains(TEXT("Gateway")))
	{
		PerceptionData.AuraColor = FVector(0.5f, 0.2f, 1.0f); // Purple for mystical
		PerceptionData.EnergyLevel += 0.5f;
		PerceptionData.bIsArchetypal = true;
	}
	else
	{
		PerceptionData.AuraColor = FVector(0.7f, 0.7f, 0.9f); // Neutral blue-white
	}
	
	// Generate symbolic meaning
	if (PerceptionData.bIsArchetypal)
	{
		PerceptionData.SymbolicMeaning = TEXT("Gateway to the Unknown");
	}
	else if (PerceptionData.EnergyLevel > 0.7f)
	{
		PerceptionData.SymbolicMeaning = TEXT("Source of Power");
	}
	else if (PerceptionData.EnergyLevel < 0.3f)
	{
		PerceptionData.SymbolicMeaning = TEXT("Dormant Potential");
	}
	else
	{
		PerceptionData.SymbolicMeaning = TEXT("Neutral Presence");
	}
	
	return PerceptionData;
}

void UPerceptionSystem::ApplyPerceptionFilters(FPerceptualObject& PerceptionData)
{
	for (const FPerceptionFilter& Filter : ActiveFilters)
	{
		if (!Filter.bActive)
		{
			continue;
		}
		
		switch (Filter.Mode)
		{
			case EPerceptionMode::Aura:
				// Enhance aura visibility
				PerceptionData.AuraColor *= (1.0f + Filter.Intensity * 0.5f);
				break;
			case EPerceptionMode::Energetic:
				// Boost energy level perception
				PerceptionData.EnergyLevel *= (1.0f + Filter.Intensity);
				break;
			case EPerceptionMode::Symbolic:
				// Enhance symbolic meaning clarity
				if (Filter.Intensity > 0.5f)
				{
					PerceptionData.SymbolicMeaning = TEXT("Clear: ") + PerceptionData.SymbolicMeaning;
				}
				break;
			case EPerceptionMode::Archetypal:
				// Reveal archetypal nature
				if (PerceptionData.EnergyLevel > 0.4f)
				{
					PerceptionData.bIsArchetypal = true;
					PerceptionData.SymbolicMeaning = TEXT("Archetypal: ") + PerceptionData.SymbolicMeaning;
				}
				break;
			case EPerceptionMode::Unity:
				// Unify all perceptions
				PerceptionData.AuraColor = FVector(1.0f, 1.0f, 1.0f);
				PerceptionData.EnergyLevel = 1.0f;
				PerceptionData.SymbolicMeaning = TEXT("One with All");
				PerceptionData.bIsArchetypal = true;
				break;
		}
	}
	
	// Apply sensitivity modifier
	PerceptionData.EnergyLevel *= (0.5f + Sensitivity * 0.5f);
}