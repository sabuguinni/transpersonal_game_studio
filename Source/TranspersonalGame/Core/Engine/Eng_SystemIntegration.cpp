#include "Eng_SystemIntegration.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogSystemIntegration);

// ===== UEng_SystemIntegration Implementation =====

void UEng_SystemIntegration::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogSystemIntegration, Log, TEXT("System Integration subsystem initializing..."));
	
	bIntegrationComplete = false;
	TotalInitializationTime = 0.0f;
	
	InitializeCriticalSystems();
	
	UE_LOG(LogSystemIntegration, Log, TEXT("System Integration subsystem initialized successfully"));
}

void UEng_SystemIntegration::Deinitialize()
{
	UE_LOG(LogSystemIntegration, Log, TEXT("System Integration subsystem deinitializing..."));
	
	// Shutdown all registered systems in reverse priority order
	TArray<FString> SystemNames;
	RegisteredSystems.GetKeys(SystemNames);
	SortSystemsByPriority(SystemNames);
	
	// Reverse the array to shutdown in reverse order
	Algo::Reverse(SystemNames);
	
	for (const FString& SystemName : SystemNames)
	{
		ShutdownSystem(SystemName);
	}
	
	RegisteredSystems.Empty();
	
	Super::Deinitialize();
}

void UEng_SystemIntegration::RegisterSystem(const FString& SystemName, EEng_IntegrationPriority Priority, const TArray<FString>& Dependencies)
{
	if (SystemName.IsEmpty())
	{
		UE_LOG(LogSystemIntegration, Warning, TEXT("Attempted to register system with empty name"));
		return;
	}
	
	if (RegisteredSystems.Contains(SystemName))
	{
		UE_LOG(LogSystemIntegration, Warning, TEXT("System already registered: %s"), *SystemName);
		return;
	}
	
	FEng_SystemIntegrationData SystemData;
	SystemData.SystemName = SystemName;
	SystemData.Priority = Priority;
	SystemData.bIsInitialized = false;
	SystemData.InitializationTime = 0.0f;
	SystemData.Dependencies = Dependencies;
	
	RegisteredSystems.Add(SystemName, SystemData);
	
	UE_LOG(LogSystemIntegration, Log, TEXT("Registered system: %s (Priority: %s, Dependencies: %d)"), 
		*SystemName, 
		*UEnum::GetValueAsString(Priority),
		Dependencies.Num());
	
	BroadcastSystemEvent(EEng_SystemEvent::ModuleLoaded, SystemName, TEXT("System registered"));
}

bool UEng_SystemIntegration::UnregisterSystem(const FString& SystemName)
{
	if (!RegisteredSystems.Contains(SystemName))
	{
		UE_LOG(LogSystemIntegration, Warning, TEXT("Attempted to unregister non-existent system: %s"), *SystemName);
		return false;
	}
	
	// Shutdown the system if it's initialized
	if (IsSystemInitialized(SystemName))
	{
		ShutdownSystem(SystemName);
	}
	
	RegisteredSystems.Remove(SystemName);
	
	UE_LOG(LogSystemIntegration, Log, TEXT("Unregistered system: %s"), *SystemName);
	BroadcastSystemEvent(EEng_SystemEvent::ModuleUnloaded, SystemName, TEXT("System unregistered"));
	
	return true;
}

bool UEng_SystemIntegration::InitializeSystem(const FString& SystemName)
{
	FEng_SystemIntegrationData* SystemData = RegisteredSystems.Find(SystemName);
	if (!SystemData)
	{
		UE_LOG(LogSystemIntegration, Warning, TEXT("Attempted to initialize non-registered system: %s"), *SystemName);
		return false;
	}
	
	if (SystemData->bIsInitialized)
	{
		UE_LOG(LogSystemIntegration, Log, TEXT("System already initialized: %s"), *SystemName);
		return true;
	}
	
	// Validate dependencies
	if (!ValidateDependencies(SystemName))
	{
		UE_LOG(LogSystemIntegration, Error, TEXT("System dependencies not met for: %s"), *SystemName);
		BroadcastSystemEvent(EEng_SystemEvent::ArchitectureViolation, SystemName, TEXT("Dependencies not met"));
		return false;
	}
	
	// Record initialization time
	float StartTime = FPlatformTime::Seconds();
	
	// Mark as initialized (actual system initialization would happen here)
	SystemData->bIsInitialized = true;
	SystemData->InitializationTime = FPlatformTime::Seconds() - StartTime;
	TotalInitializationTime += SystemData->InitializationTime;
	
	UE_LOG(LogSystemIntegration, Log, TEXT("Initialized system: %s (%.3f ms)"), 
		*SystemName, SystemData->InitializationTime * 1000.0f);
	
	BroadcastSystemEvent(EEng_SystemEvent::SystemInitialized, SystemName, TEXT("System initialized successfully"));
	
	return true;
}

bool UEng_SystemIntegration::ShutdownSystem(const FString& SystemName)
{
	FEng_SystemIntegrationData* SystemData = RegisteredSystems.Find(SystemName);
	if (!SystemData)
	{
		UE_LOG(LogSystemIntegration, Warning, TEXT("Attempted to shutdown non-registered system: %s"), *SystemName);
		return false;
	}
	
	if (!SystemData->bIsInitialized)
	{
		UE_LOG(LogSystemIntegration, Log, TEXT("System not initialized, no shutdown needed: %s"), *SystemName);
		return true;
	}
	
	// Mark as shutdown (actual system shutdown would happen here)
	SystemData->bIsInitialized = false;
	
	UE_LOG(LogSystemIntegration, Log, TEXT("Shutdown system: %s"), *SystemName);
	BroadcastSystemEvent(EEng_SystemEvent::SystemShutdown, SystemName, TEXT("System shutdown"));
	
	return true;
}

bool UEng_SystemIntegration::IsSystemRegistered(const FString& SystemName) const
{
	return RegisteredSystems.Contains(SystemName);
}

bool UEng_SystemIntegration::IsSystemInitialized(const FString& SystemName) const
{
	const FEng_SystemIntegrationData* SystemData = RegisteredSystems.Find(SystemName);
	return SystemData && SystemData->bIsInitialized;
}

TArray<FString> UEng_SystemIntegration::GetSystemDependencies(const FString& SystemName) const
{
	const FEng_SystemIntegrationData* SystemData = RegisteredSystems.Find(SystemName);
	if (SystemData)
	{
		return SystemData->Dependencies;
	}
	return TArray<FString>();
}

bool UEng_SystemIntegration::ValidateSystemIntegration()
{
	UE_LOG(LogSystemIntegration, Log, TEXT("Validating system integration..."));
	
	bool bAllValid = true;
	int32 InitializedCount = 0;
	
	for (const auto& SystemPair : RegisteredSystems)
	{
		const FString& SystemName = SystemPair.Key;
		const FEng_SystemIntegrationData& SystemData = SystemPair.Value;
		
		// Check if critical systems are initialized
		if (SystemData.Priority == EEng_IntegrationPriority::Critical && !SystemData.bIsInitialized)
		{
			UE_LOG(LogSystemIntegration, Error, TEXT("Critical system not initialized: %s"), *SystemName);
			BroadcastSystemEvent(EEng_SystemEvent::ArchitectureViolation, SystemName, TEXT("Critical system not initialized"));
			bAllValid = false;
		}
		
		// Validate dependencies
		if (!ValidateDependencies(SystemName))
		{
			UE_LOG(LogSystemIntegration, Error, TEXT("System dependency validation failed: %s"), *SystemName);
			bAllValid = false;
		}
		
		if (SystemData.bIsInitialized)
		{
			InitializedCount++;
		}
	}
	
	bIntegrationComplete = bAllValid && (InitializedCount > 0);
	
	UE_LOG(LogSystemIntegration, Log, TEXT("Integration validation complete. Valid: %s, Initialized: %d/%d"), 
		bAllValid ? TEXT("Yes") : TEXT("No"), InitializedCount, RegisteredSystems.Num());
	
	if (bIntegrationComplete)
	{
		BroadcastSystemEvent(EEng_SystemEvent::IntegrationComplete, TEXT("All"), TEXT("System integration validation passed"));
	}
	
	return bAllValid;
}

void UEng_SystemIntegration::LogIntegrationStatus() const
{
	UE_LOG(LogSystemIntegration, Log, TEXT("=== SYSTEM INTEGRATION STATUS ==="));
	UE_LOG(LogSystemIntegration, Log, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
	UE_LOG(LogSystemIntegration, Log, TEXT("Integration Complete: %s"), bIntegrationComplete ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogSystemIntegration, Log, TEXT("Total Initialization Time: %.3f ms"), TotalInitializationTime * 1000.0f);
	
	for (const auto& SystemPair : RegisteredSystems)
	{
		const FEng_SystemIntegrationData& SystemData = SystemPair.Value;
		UE_LOG(LogSystemIntegration, Log, TEXT("  System: %s | Priority: %s | Initialized: %s | Time: %.3f ms | Dependencies: %d"), 
			*SystemData.SystemName,
			*UEnum::GetValueAsString(SystemData.Priority),
			SystemData.bIsInitialized ? TEXT("Yes") : TEXT("No"),
			SystemData.InitializationTime * 1000.0f,
			SystemData.Dependencies.Num());
	}
}

void UEng_SystemIntegration::BroadcastSystemEvent(EEng_SystemEvent EventType, const FString& SystemName, const FString& Message)
{
	UE_LOG(LogSystemIntegration, Log, TEXT("System Event: %s | System: %s | Message: %s"), 
		*UEnum::GetValueAsString(EventType), *SystemName, *Message);
	
	// Here we could implement a delegate broadcast system for other systems to listen to events
}

void UEng_SystemIntegration::InitializeCriticalSystems()
{
	// Register critical core systems
	RegisterSystem(TEXT("CoreArchitecture"), EEng_IntegrationPriority::Critical, TArray<FString>());
	RegisterSystem(TEXT("WorldGeneration"), EEng_IntegrationPriority::Critical, {TEXT("CoreArchitecture")});
	RegisterSystem(TEXT("CharacterSystems"), EEng_IntegrationPriority::High, {TEXT("CoreArchitecture")});
	RegisterSystem(TEXT("DinosaurAI"), EEng_IntegrationPriority::High, {TEXT("CoreArchitecture"), TEXT("CharacterSystems")});
	RegisterSystem(TEXT("CombatSystems"), EEng_IntegrationPriority::Medium, {TEXT("CharacterSystems"), TEXT("DinosaurAI")});
	RegisterSystem(TEXT("QuestSystems"), EEng_IntegrationPriority::Medium, {TEXT("CharacterSystems")});
	RegisterSystem(TEXT("AudioSystems"), EEng_IntegrationPriority::Low, TArray<FString>());
	RegisterSystem(TEXT("VFXSystems"), EEng_IntegrationPriority::Low, TArray<FString>());
	RegisterSystem(TEXT("UISystems"), EEng_IntegrationPriority::Background, TArray<FString>());
}

bool UEng_SystemIntegration::ValidateDependencies(const FString& SystemName) const
{
	const FEng_SystemIntegrationData* SystemData = RegisteredSystems.Find(SystemName);
	if (!SystemData)
	{
		return false;
	}
	
	for (const FString& Dependency : SystemData->Dependencies)
	{
		if (!IsSystemInitialized(Dependency))
		{
			UE_LOG(LogSystemIntegration, Warning, TEXT("Dependency not initialized: %s (required by %s)"), 
				*Dependency, *SystemName);
			return false;
		}
	}
	
	return true;
}

void UEng_SystemIntegration::SortSystemsByPriority(TArray<FString>& SystemNames) const
{
	SystemNames.Sort([this](const FString& A, const FString& B) {
		const FEng_SystemIntegrationData* DataA = RegisteredSystems.Find(A);
		const FEng_SystemIntegrationData* DataB = RegisteredSystems.Find(B);
		
		if (!DataA || !DataB)
		{
			return false;
		}
		
		return (int32)DataA->Priority > (int32)DataB->Priority;
	});
}

// ===== UEng_SystemIntegrationComponent Implementation =====

UEng_SystemIntegrationComponent::UEng_SystemIntegrationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIntegrationValid = false;
	IntegrationStartTime = 0.0f;
}

void UEng_SystemIntegrationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	IntegrationStartTime = FPlatformTime::Seconds();
	InitializeIntegrationComponent();
}

void UEng_SystemIntegrationComponent::RegisterActorWithSystems()
{
	UEng_SystemIntegration* IntegrationSystem = GetWorld()->GetGameInstance()->GetSubsystem<UEng_SystemIntegration>();
	if (IntegrationSystem)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			UE_LOG(LogSystemIntegration, Log, TEXT("Registered actor %s with integration system"), *Owner->GetName());
		}
	}
}

bool UEng_SystemIntegrationComponent::ValidateActorIntegration()
{
	UEng_SystemIntegration* IntegrationSystem = GetWorld()->GetGameInstance()->GetSubsystem<UEng_SystemIntegration>();
	if (!IntegrationSystem)
	{
		UE_LOG(LogSystemIntegration, Warning, TEXT("Integration system not available"));
		return false;
	}
	
	bool bAllRequiredSystemsAvailable = true;
	
	for (const FString& RequiredSystem : RequiredSystems)
	{
		if (!IntegrationSystem->IsSystemInitialized(RequiredSystem))
		{
			UE_LOG(LogSystemIntegration, Warning, TEXT("Required system not available: %s"), *RequiredSystem);
			bAllRequiredSystemsAvailable = false;
		}
	}
	
	bIntegrationValid = bAllRequiredSystemsAvailable;
	return bIntegrationValid;
}

void UEng_SystemIntegrationComponent::NotifySystemDependency(const FString& SystemName)
{
	if (!SystemName.IsEmpty() && !RequiredSystems.Contains(SystemName))
	{
		RequiredSystems.Add(SystemName);
		UE_LOG(LogSystemIntegration, Log, TEXT("Added system dependency: %s"), *SystemName);
	}
}

void UEng_SystemIntegrationComponent::InitializeIntegrationComponent()
{
	UE_LOG(LogSystemIntegration, Log, TEXT("Initializing System Integration Component..."));
	
	CheckSystemAvailability();
	ValidateRequiredSystems();
	RegisterActorWithSystems();
	
	float InitTime = FPlatformTime::Seconds() - IntegrationStartTime;
	UE_LOG(LogSystemIntegration, Log, TEXT("System Integration Component initialized in %.3f ms. Valid: %s"), 
		InitTime * 1000.0f, bIntegrationValid ? TEXT("Yes") : TEXT("No"));
}

void UEng_SystemIntegrationComponent::CheckSystemAvailability()
{
	UEng_SystemIntegration* IntegrationSystem = GetWorld()->GetGameInstance()->GetSubsystem<UEng_SystemIntegration>();
	if (IntegrationSystem)
	{
		UE_LOG(LogSystemIntegration, Log, TEXT("System Integration subsystem is available"));
	}
	else
	{
		UE_LOG(LogSystemIntegration, Warning, TEXT("System Integration subsystem not available"));
	}
}

void UEng_SystemIntegrationComponent::ValidateRequiredSystems()
{
	if (RequiredSystems.Num() == 0)
	{
		UE_LOG(LogSystemIntegration, Log, TEXT("No required systems specified"));
		bIntegrationValid = true;
		return;
	}
	
	ValidateActorIntegration();
}