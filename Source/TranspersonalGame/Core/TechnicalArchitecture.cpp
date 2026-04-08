// Copyright Epic Games, Inc. All Rights Reserved.

#include "TechnicalArchitecture.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/RendererSettings.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "WorldPartition/WorldPartition.h"
#include "Rendering/NaniteResources.h"

DEFINE_LOG_CATEGORY_STATIC(LogTechnicalArchitecture, Log, All);

void UTechnicalArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTechnicalArchitecture, Log, TEXT("Initializing Technical Architecture Subsystem"));
	
	InitializeFeatureValidation();
	
	// Enforce core architecture requirements
	EnforceNaniteRequirements();
	EnforceLumenSettings();
	EnforceVSMConfiguration();
	EnforceWorldPartitionSettings();
	
	// Log current architecture status
	LogArchitectureStatus();
	
	UE_LOG(LogTechnicalArchitecture, Log, TEXT("Technical Architecture Subsystem initialized successfully"));
}

void UTechnicalArchitectureSubsystem::Deinitialize()
{
	UE_LOG(LogTechnicalArchitecture, Log, TEXT("Deinitializing Technical Architecture Subsystem"));
	
	RegisteredModules.Empty();
	FeatureStatus.Empty();
	
	Super::Deinitialize();
}

bool UTechnicalArchitectureSubsystem::RegisterModule(const FModuleInfo& ModuleInfo)
{
	// Validate module info
	if (ModuleInfo.ModuleName.IsEmpty() || ModuleInfo.ResponsibleAgent.IsEmpty())
	{
		UE_LOG(LogTechnicalArchitecture, Warning, TEXT("Cannot register module with empty name or agent"));
		return false;
	}
	
	// Check if module already registered
	for (const FModuleInfo& ExistingModule : RegisteredModules)
	{
		if (ExistingModule.ModuleName == ModuleInfo.ModuleName)
		{
			UE_LOG(LogTechnicalArchitecture, Warning, TEXT("Module %s already registered"), *ModuleInfo.ModuleName);
			return false;
		}
	}
	
	// Add to registered modules
	RegisteredModules.Add(ModuleInfo);
	
	UE_LOG(LogTechnicalArchitecture, Log, TEXT("Registered module: %s (Agent: %s)"), 
		*ModuleInfo.ModuleName, *ModuleInfo.ResponsibleAgent);
	
	return true;
}

bool UTechnicalArchitectureSubsystem::UnregisterModule(const FString& ModuleName)
{
	for (int32 i = RegisteredModules.Num() - 1; i >= 0; i--)
	{
		if (RegisteredModules[i].ModuleName == ModuleName)
		{
			RegisteredModules.RemoveAt(i);
			UE_LOG(LogTechnicalArchitecture, Log, TEXT("Unregistered module: %s"), *ModuleName);
			return true;
		}
	}
	
	UE_LOG(LogTechnicalArchitecture, Warning, TEXT("Module %s not found for unregistration"), *ModuleName);
	return false;
}

TArray<FModuleInfo> UTechnicalArchitectureSubsystem::GetRegisteredModules() const
{
	return RegisteredModules;
}

bool UTechnicalArchitectureSubsystem::ValidateRequiredFeatures()
{
	bool bAllFeaturesValid = true;
	
	// Check Nanite
	if (!IsFeatureEnabled(ETechnicalFeature::Nanite))
	{
		UE_LOG(LogTechnicalArchitecture, Error, TEXT("Nanite is required but not enabled"));
		bAllFeaturesValid = false;
	}
	
	// Check Lumen
	if (!IsFeatureEnabled(ETechnicalFeature::Lumen))
	{
		UE_LOG(LogTechnicalArchitecture, Error, TEXT("Lumen is required but not enabled"));
		bAllFeaturesValid = false;
	}
	
	// Check Virtual Shadow Maps
	if (!IsFeatureEnabled(ETechnicalFeature::VSM))
	{
		UE_LOG(LogTechnicalArchitecture, Error, TEXT("Virtual Shadow Maps are required but not enabled"));
		bAllFeaturesValid = false;
	}
	
	// Check World Partition
	if (!IsFeatureEnabled(ETechnicalFeature::WorldPartition))
	{
		UE_LOG(LogTechnicalArchitecture, Error, TEXT("World Partition is required but not enabled"));
		bAllFeaturesValid = false;
	}
	
	return bAllFeaturesValid;
}

bool UTechnicalArchitectureSubsystem::IsFeatureEnabled(ETechnicalFeature Feature) const
{
	const bool* FeaturePtr = FeatureStatus.Find(Feature);
	return FeaturePtr ? *FeaturePtr : false;
}

float UTechnicalArchitectureSubsystem::GetCurrentFramerate() const
{
	if (GEngine && GEngine->GetGameViewport())
	{
		return 1.0f / GEngine->GetGameViewport()->GetWorld()->GetDeltaSeconds();
	}
	return 0.0f;
}

bool UTechnicalArchitectureSubsystem::IsPerformanceWithinTargets() const
{
	float CurrentFPS = GetCurrentFramerate();
	
	// Check if we're meeting our target framerate
	bool bFramerateOK = CurrentFPS >= TPSArchitecture::TARGET_FRAMERATE_CONSOLE;
	
	// Check memory usage
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	float MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
	bool bMemoryOK = MemoryUsageMB <= TPSArchitecture::TARGET_MEMORY_CONSOLE_MB;
	
	return bFramerateOK && bMemoryOK;
}

bool UTechnicalArchitectureSubsystem::ValidateArchitectureCompliance()
{
	bool bCompliant = true;
	
	// Validate required features
	if (!ValidateRequiredFeatures())
	{
		bCompliant = false;
	}
	
	// Validate module dependencies
	if (!ValidateModuleDependencies())
	{
		bCompliant = false;
	}
	
	// Validate performance targets
	if (!IsPerformanceWithinTargets())
	{
		UE_LOG(LogTechnicalArchitecture, Warning, TEXT("Performance not within targets"));
		// Don't fail compliance for performance, just warn
	}
	
	return bCompliant;
}

void UTechnicalArchitectureSubsystem::InitializeFeatureValidation()
{
	FeatureStatus.Empty();
	
	// Check Nanite
	const URendererSettings* RendererSettings = GetDefault<URendererSettings>();
	FeatureStatus.Add(ETechnicalFeature::Nanite, RendererSettings && RendererSettings->bEnableNanite);
	
	// Check Lumen
	bool bLumenEnabled = RendererSettings && 
		RendererSettings->DynamicGlobalIlluminationMethod == EDynamicGlobalIlluminationMethod::Lumen;
	FeatureStatus.Add(ETechnicalFeature::Lumen, bLumenEnabled);
	
	// Check Virtual Shadow Maps
	bool bVSMEnabled = RendererSettings && RendererSettings->ShadowMapMethod == EShadowMapMethod::VirtualShadowMaps;
	FeatureStatus.Add(ETechnicalFeature::VSM, bVSMEnabled);
	
	// Check World Partition (requires world context)
	UWorld* World = GetWorld();
	bool bWorldPartitionEnabled = World && World->IsPartitionedWorld();
	FeatureStatus.Add(ETechnicalFeature::WorldPartition, bWorldPartitionEnabled);
	
	// Check Mass Entity (plugin-based)
	FeatureStatus.Add(ETechnicalFeature::MassEntity, true); // Assume enabled for now
	
	// Check Chaos Physics
	FeatureStatus.Add(ETechnicalFeature::ChaosPhysics, true); // Default in UE5
	
	// Check MetaSounds
	FeatureStatus.Add(ETechnicalFeature::MetaSounds, true); // Assume enabled
	
	// Check Niagara
	FeatureStatus.Add(ETechnicalFeature::Niagara, true); // Default in UE5
}

void UTechnicalArchitectureSubsystem::EnforceNaniteRequirements()
{
	URendererSettings* RendererSettings = GetMutableDefault<URendererSettings>();
	if (RendererSettings)
	{
		if (!RendererSettings->bEnableNanite)
		{
			UE_LOG(LogTechnicalArchitecture, Warning, TEXT("Enabling Nanite as required by architecture"));
			RendererSettings->bEnableNanite = true;
		}
		
		// Ensure mesh distance fields are enabled for Lumen
		if (!RendererSettings->bGenerateMeshDistanceFields)
		{
			UE_LOG(LogTechnicalArchitecture, Warning, TEXT("Enabling Mesh Distance Fields for Lumen"));
			RendererSettings->bGenerateMeshDistanceFields = true;
		}
	}
}

void UTechnicalArchitectureSubsystem::EnforceLumenSettings()
{
	URendererSettings* RendererSettings = GetMutableDefault<URendererSettings>();
	if (RendererSettings)
	{
		// Enable Lumen Global Illumination
		if (RendererSettings->DynamicGlobalIlluminationMethod != EDynamicGlobalIlluminationMethod::Lumen)
		{
			UE_LOG(LogTechnicalArchitecture, Warning, TEXT("Setting Lumen as Global Illumination method"));
			RendererSettings->DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Lumen;
		}
		
		// Enable Lumen Reflections
		if (RendererSettings->ReflectionMethod != EReflectionMethod::Lumen)
		{
			UE_LOG(LogTechnicalArchitecture, Warning, TEXT("Setting Lumen as Reflection method"));
			RendererSettings->ReflectionMethod = EReflectionMethod::Lumen;
		}
		
		// Disable static lighting
		if (RendererSettings->bAllowStaticLighting)
		{
			UE_LOG(LogTechnicalArchitecture, Warning, TEXT("Disabling static lighting for Lumen"));
			RendererSettings->bAllowStaticLighting = false;
		}
	}
}

void UTechnicalArchitectureSubsystem::EnforceVSMConfiguration()
{
	URendererSettings* RendererSettings = GetMutableDefault<URendererSettings>();
	if (RendererSettings)
	{
		// Enable Virtual Shadow Maps
		if (RendererSettings->ShadowMapMethod != EShadowMapMethod::VirtualShadowMaps)
		{
			UE_LOG(LogTechnicalArchitecture, Warning, TEXT("Setting Virtual Shadow Maps as shadow method"));
			RendererSettings->ShadowMapMethod = EShadowMapMethod::VirtualShadowMaps;
		}
	}
}

void UTechnicalArchitectureSubsystem::EnforceWorldPartitionSettings()
{
	// World Partition settings are per-world and cannot be enforced globally
	// This will be validated when worlds are loaded
	UE_LOG(LogTechnicalArchitecture, Log, TEXT("World Partition enforcement requires per-world validation"));
}

bool UTechnicalArchitectureSubsystem::ValidateModuleDependencies() const
{
	// Check that all module dependencies are satisfied
	for (const FModuleInfo& Module : RegisteredModules)
	{
		for (const FString& Dependency : Module.Dependencies)
		{
			bool bDependencyFound = false;
			for (const FModuleInfo& OtherModule : RegisteredModules)
			{
				if (OtherModule.ModuleName == Dependency)
				{
					bDependencyFound = true;
					break;
				}
			}
			
			if (!bDependencyFound)
			{
				UE_LOG(LogTechnicalArchitecture, Error, 
					TEXT("Module %s has unresolved dependency: %s"), 
					*Module.ModuleName, *Dependency);
				return false;
			}
		}
	}
	
	return true;
}

void UTechnicalArchitectureSubsystem::LogArchitectureStatus() const
{
	UE_LOG(LogTechnicalArchitecture, Log, TEXT("=== TECHNICAL ARCHITECTURE STATUS ==="));
	
	// Log feature status
	for (const auto& FeaturePair : FeatureStatus)
	{
		FString FeatureName = UEnum::GetValueAsString(FeaturePair.Key);
		FString Status = FeaturePair.Value ? TEXT("ENABLED") : TEXT("DISABLED");
		UE_LOG(LogTechnicalArchitecture, Log, TEXT("%s: %s"), *FeatureName, *Status);
	}
	
	// Log registered modules
	UE_LOG(LogTechnicalArchitecture, Log, TEXT("Registered Modules: %d"), RegisteredModules.Num());
	for (const FModuleInfo& Module : RegisteredModules)
	{
		UE_LOG(LogTechnicalArchitecture, Log, TEXT("  - %s (Agent: %s)"), 
			*Module.ModuleName, *Module.ResponsibleAgent);
	}
	
	// Log performance status
	float CurrentFPS = GetCurrentFramerate();
	bool bPerformanceOK = IsPerformanceWithinTargets();
	UE_LOG(LogTechnicalArchitecture, Log, TEXT("Current FPS: %.1f, Performance OK: %s"), 
		CurrentFPS, bPerformanceOK ? TEXT("YES") : TEXT("NO"));
	
	UE_LOG(LogTechnicalArchitecture, Log, TEXT("=== END ARCHITECTURE STATUS ==="));
}