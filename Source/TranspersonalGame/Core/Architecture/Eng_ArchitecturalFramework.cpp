#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalFramework initialized"));
	
	InitializeCoreSystems();
	EnforceArchitecturalRules();
}

void UEng_ArchitecturalFramework::RegisterSystem(const FString& SystemName, EEng_ArchitecturalLayer Layer, EEng_SystemPriority Priority)
{
	FEng_SystemDependency NewSystem;
	NewSystem.SystemName = SystemName;
	NewSystem.Layer = Layer;
	NewSystem.Priority = Priority;
	
	// Check if system already exists
	for (int32 i = 0; i < RegisteredSystems.Num(); i++)
	{
		if (RegisteredSystems[i].SystemName == SystemName)
		{
			RegisteredSystems[i] = NewSystem;
			UE_LOG(LogTemp, Warning, TEXT("Updated existing system: %s"), *SystemName);
			return;
		}
	}
	
	RegisteredSystems.Add(NewSystem);
	UE_LOG(LogTemp, Warning, TEXT("Registered new system: %s in layer %d"), *SystemName, (int32)Layer);
}

void UEng_ArchitecturalFramework::AddSystemDependency(const FString& SystemName, const FString& DependencyName)
{
	for (FEng_SystemDependency& System : RegisteredSystems)
	{
		if (System.SystemName == SystemName)
		{
			System.Dependencies.AddUnique(DependencyName);
			UE_LOG(LogTemp, Warning, TEXT("Added dependency %s to system %s"), *DependencyName, *SystemName);
			return;
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("System %s not found when adding dependency"), *SystemName);
}

TArray<FEng_SystemDependency> UEng_ArchitecturalFramework::GetSystemsByLayer(EEng_ArchitecturalLayer Layer) const
{
	TArray<FEng_SystemDependency> LayerSystems;
	for (const FEng_SystemDependency& System : RegisteredSystems)
	{
		if (System.Layer == Layer)
		{
			LayerSystems.Add(System);
		}
	}
	return LayerSystems;
}

bool UEng_ArchitecturalFramework::ValidateSystemDependencies() const
{
	bool bIsValid = true;
	
	// Check for circular dependencies
	if (!CheckCircularDependencies())
	{
		UE_LOG(LogTemp, Error, TEXT("Circular dependencies detected in system architecture"));
		bIsValid = false;
	}
	
	// Validate layer hierarchy
	ValidateLayerHierarchy();
	
	return bIsValid;
}

void UEng_ArchitecturalFramework::EnforceArchitecturalRules()
{
	UE_LOG(LogTemp, Warning, TEXT("Enforcing architectural rules..."));
	
	// Rule 1: Foundation systems cannot depend on higher layers
	TArray<FEng_SystemDependency> FoundationSystems = GetSystemsByLayer(EEng_ArchitecturalLayer::Foundation);
	for (const FEng_SystemDependency& System : FoundationSystems)
	{
		for (const FString& Dependency : System.Dependencies)
		{
			for (const FEng_SystemDependency& DepSystem : RegisteredSystems)
			{
				if (DepSystem.SystemName == Dependency && DepSystem.Layer != EEng_ArchitecturalLayer::Foundation)
				{
					UE_LOG(LogTemp, Error, TEXT("VIOLATION: Foundation system %s depends on higher layer system %s"), 
						*System.SystemName, *Dependency);
				}
			}
		}
	}
	
	// Rule 2: Critical systems must be in Foundation or Core layers
	for (const FEng_SystemDependency& System : RegisteredSystems)
	{
		if (System.Priority == EEng_SystemPriority::Critical)
		{
			if (System.Layer != EEng_ArchitecturalLayer::Foundation && System.Layer != EEng_ArchitecturalLayer::Core)
			{
				UE_LOG(LogTemp, Error, TEXT("VIOLATION: Critical system %s is not in Foundation or Core layer"), *System.SystemName);
			}
		}
	}
}

void UEng_ArchitecturalFramework::GenerateArchitectureReport()
{
	UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL FRAMEWORK REPORT ==="));
	UE_LOG(LogTemp, Warning, TEXT("Total registered systems: %d"), RegisteredSystems.Num());
	
	for (int32 LayerIndex = 0; LayerIndex < 4; LayerIndex++)
	{
		EEng_ArchitecturalLayer Layer = (EEng_ArchitecturalLayer)LayerIndex;
		TArray<FEng_SystemDependency> LayerSystems = GetSystemsByLayer(Layer);
		
		FString LayerName;
		switch (Layer)
		{
			case EEng_ArchitecturalLayer::Foundation: LayerName = TEXT("Foundation"); break;
			case EEng_ArchitecturalLayer::Core: LayerName = TEXT("Core"); break;
			case EEng_ArchitecturalLayer::Gameplay: LayerName = TEXT("Gameplay"); break;
			case EEng_ArchitecturalLayer::Presentation: LayerName = TEXT("Presentation"); break;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("--- %s Layer (%d systems) ---"), *LayerName, LayerSystems.Num());
		for (const FEng_SystemDependency& System : LayerSystems)
		{
			UE_LOG(LogTemp, Warning, TEXT("  %s (Priority: %d, Dependencies: %d)"), 
				*System.SystemName, (int32)System.Priority, System.Dependencies.Num());
		}
	}
	
	ValidateSystemDependencies();
}

void UEng_ArchitecturalFramework::InitializeCoreSystems()
{
	// Register core architectural systems
	RegisterSystem(TEXT("PhysicsCore"), EEng_ArchitecturalLayer::Foundation, EEng_SystemPriority::Critical);
	RegisterSystem(TEXT("WorldGeneration"), EEng_ArchitecturalLayer::Core, EEng_SystemPriority::High);
	RegisterSystem(TEXT("CharacterSystem"), EEng_ArchitecturalLayer::Gameplay, EEng_SystemPriority::High);
	RegisterSystem(TEXT("DinosaurAI"), EEng_ArchitecturalLayer::Gameplay, EEng_SystemPriority::Medium);
	RegisterSystem(TEXT("RenderingSystem"), EEng_ArchitecturalLayer::Presentation, EEng_SystemPriority::Medium);
	
	// Add dependencies
	AddSystemDependency(TEXT("WorldGeneration"), TEXT("PhysicsCore"));
	AddSystemDependency(TEXT("CharacterSystem"), TEXT("PhysicsCore"));
	AddSystemDependency(TEXT("DinosaurAI"), TEXT("CharacterSystem"));
}

bool UEng_ArchitecturalFramework::CheckCircularDependencies() const
{
	// Simple circular dependency check - can be enhanced
	for (const FEng_SystemDependency& System : RegisteredSystems)
	{
		for (const FString& Dependency : System.Dependencies)
		{
			// Check if dependency also depends on this system
			for (const FEng_SystemDependency& DepSystem : RegisteredSystems)
			{
				if (DepSystem.SystemName == Dependency)
				{
					if (DepSystem.Dependencies.Contains(System.SystemName))
					{
						UE_LOG(LogTemp, Error, TEXT("Circular dependency detected: %s <-> %s"), 
							*System.SystemName, *Dependency);
						return false;
					}
				}
			}
		}
	}
	return true;
}

void UEng_ArchitecturalFramework::ValidateLayerHierarchy() const
{
	// Validate that systems only depend on same or lower layers
	for (const FEng_SystemDependency& System : RegisteredSystems)
	{
		for (const FString& Dependency : System.Dependencies)
		{
			for (const FEng_SystemDependency& DepSystem : RegisteredSystems)
			{
				if (DepSystem.SystemName == Dependency)
				{
					if ((int32)DepSystem.Layer > (int32)System.Layer)
					{
						UE_LOG(LogTemp, Error, TEXT("Layer violation: %s (layer %d) depends on %s (layer %d)"), 
							*System.SystemName, (int32)System.Layer, *Dependency, (int32)DepSystem.Layer);
					}
				}
			}
		}
	}
}