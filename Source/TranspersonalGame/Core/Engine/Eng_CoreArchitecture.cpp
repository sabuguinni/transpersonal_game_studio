#include "Eng_CoreArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogEngineArchitect);

// ===== UEng_CoreArchitecture Implementation =====

void UEng_CoreArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogEngineArchitect, Log, TEXT("Engine Core Architecture subsystem initializing..."));
	
	MaxActorLimit = 8000; // Global actor limit
	LastFrameTime = 0.0f;
	
	InitializeCoreModules();
	ValidatePerformanceLimits();
	
	UE_LOG(LogEngineArchitect, Log, TEXT("Engine Core Architecture subsystem initialized successfully"));
}

void UEng_CoreArchitecture::Deinitialize()
{
	UE_LOG(LogEngineArchitect, Log, TEXT("Engine Core Architecture subsystem deinitializing..."));
	
	LoadedModules.Empty();
	
	Super::Deinitialize();
}

bool UEng_CoreArchitecture::ValidateSystemDependencies()
{
	UE_LOG(LogEngineArchitect, Log, TEXT("Validating system dependencies..."));
	
	// Check critical modules
	TArray<FString> CriticalModules = {
		TEXT("TranspersonalGame"),
		TEXT("CoreSystems"),
		TEXT("WorldGeneration"),
		TEXT("CharacterSystems")
	};
	
	bool bAllValid = true;
	for (const FString& Module : CriticalModules)
	{
		if (!IsModuleLoaded(Module))
		{
			UE_LOG(LogEngineArchitect, Warning, TEXT("Critical module not loaded: %s"), *Module);
			bAllValid = false;
		}
	}
	
	return bAllValid;
}

void UEng_CoreArchitecture::RegisterCoreModule(const FString& ModuleName, int32 Priority)
{
	if (ModuleName.IsEmpty())
	{
		UE_LOG(LogEngineArchitect, Warning, TEXT("Attempted to register empty module name"));
		return;
	}
	
	LoadedModules.Add(ModuleName, Priority);
	UE_LOG(LogEngineArchitect, Log, TEXT("Registered module: %s (Priority: %d)"), *ModuleName, Priority);
}

bool UEng_CoreArchitecture::IsModuleLoaded(const FString& ModuleName) const
{
	return LoadedModules.Contains(ModuleName);
}

float UEng_CoreArchitecture::GetCurrentFrameTime() const
{
	if (GEngine && GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
	{
		return GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull)->GetDeltaSeconds();
	}
	return LastFrameTime;
}

int32 UEng_CoreArchitecture::GetActiveActorCount() const
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (World)
	{
		return World->GetActorCount();
	}
	return 0;
}

void UEng_CoreArchitecture::LogSystemStatus() const
{
	UE_LOG(LogEngineArchitect, Log, TEXT("=== ENGINE ARCHITECTURE STATUS ==="));
	UE_LOG(LogEngineArchitect, Log, TEXT("Loaded Modules: %d"), LoadedModules.Num());
	UE_LOG(LogEngineArchitect, Log, TEXT("Active Actors: %d / %d"), GetActiveActorCount(), MaxActorLimit);
	UE_LOG(LogEngineArchitect, Log, TEXT("Frame Time: %.3f ms"), GetCurrentFrameTime() * 1000.0f);
	
	for (const auto& Module : LoadedModules)
	{
		UE_LOG(LogEngineArchitect, Log, TEXT("  Module: %s (Priority: %d)"), *Module.Key, Module.Value);
	}
}

void UEng_CoreArchitecture::InitializeCoreModules()
{
	// Register core modules with priorities
	RegisterCoreModule(TEXT("TranspersonalGame"), 1000);
	RegisterCoreModule(TEXT("CoreSystems"), 900);
	RegisterCoreModule(TEXT("WorldGeneration"), 800);
	RegisterCoreModule(TEXT("CharacterSystems"), 700);
	RegisterCoreModule(TEXT("DinosaurAI"), 600);
	RegisterCoreModule(TEXT("CombatSystems"), 500);
	RegisterCoreModule(TEXT("QuestSystems"), 400);
	RegisterCoreModule(TEXT("AudioSystems"), 300);
	RegisterCoreModule(TEXT("VFXSystems"), 200);
	RegisterCoreModule(TEXT("UISystems"), 100);
}

void UEng_CoreArchitecture::ValidatePerformanceLimits()
{
	int32 CurrentActors = GetActiveActorCount();
	if (CurrentActors > MaxActorLimit)
	{
		UE_LOG(LogEngineArchitect, Warning, TEXT("Actor count exceeds limit: %d / %d"), CurrentActors, MaxActorLimit);
	}
}

// ===== UEng_ArchitectureComponent Implementation =====

UEng_ArchitectureComponent::UEng_ArchitectureComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bValidateOnBeginPlay = true;
	bIsArchitectureValid = false;
	ComponentVersion = TEXT("1.0.0");
}

void UEng_ArchitectureComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (bValidateOnBeginPlay)
	{
		InitializeArchitectureComponent();
	}
}

bool UEng_ArchitectureComponent::ValidateActorArchitecture()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogEngineArchitect, Warning, TEXT("ArchitectureComponent has no valid owner"));
		return false;
	}
	
	// Validate actor has required components
	bool bHasValidSetup = true;
	
	// Check for basic requirements
	if (!Owner->GetRootComponent())
	{
		UE_LOG(LogEngineArchitect, Warning, TEXT("Actor %s missing root component"), *Owner->GetName());
		bHasValidSetup = false;
	}
	
	bIsArchitectureValid = bHasValidSetup;
	return bIsArchitectureValid;
}

void UEng_ArchitectureComponent::RegisterWithArchitectureSystem()
{
	UEng_CoreArchitecture* ArchSystem = GetWorld()->GetGameInstance()->GetSubsystem<UEng_CoreArchitecture>();
	if (ArchSystem)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			UE_LOG(LogEngineArchitect, Log, TEXT("Registered actor %s with architecture system"), *Owner->GetName());
		}
	}
}

void UEng_ArchitectureComponent::InitializeArchitectureComponent()
{
	UE_LOG(LogEngineArchitect, Log, TEXT("Initializing Architecture Component v%s"), *ComponentVersion);
	
	PerformValidationChecks();
	RegisterWithArchitectureSystem();
	
	UE_LOG(LogEngineArchitect, Log, TEXT("Architecture Component initialization complete. Valid: %s"), 
		bIsArchitectureValid ? TEXT("Yes") : TEXT("No"));
}

void UEng_ArchitectureComponent::PerformValidationChecks()
{
	bIsArchitectureValid = ValidateActorArchitecture();
	
	if (bIsArchitectureValid)
	{
		UE_LOG(LogEngineArchitect, Log, TEXT("Actor architecture validation passed"));
	}
	else
	{
		UE_LOG(LogEngineArchitect, Warning, TEXT("Actor architecture validation failed"));
	}
}