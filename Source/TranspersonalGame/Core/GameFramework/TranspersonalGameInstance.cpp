#include "TranspersonalGameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"
#include "../StudioDirectorSubsystem.h"

UTranspersonalGameInstance::UTranspersonalGameInstance()
{
	// Initialize game instance defaults
	bIsGameInitialized = false;
	GameStartTime = 0.0f;
	CurrentGamePhase = EGamePhase::MainMenu;
}

void UTranspersonalGameInstance::Init()
{
	Super::Init();
	
	UE_LOG(LogTranspersonalGame, Warning, TEXT("TranspersonalGameInstance: Initializing"));
	
	// Initialize Studio Director subsystem
	InitializeStudioDirector();
	
	// Setup game-specific configurations
	SetupGameConfiguration();
	
	bIsGameInitialized = true;
	GameStartTime = FPlatformTime::Seconds();
	
	UE_LOG(LogTranspersonalGame, Warning, TEXT("TranspersonalGameInstance: Initialization Complete"));
}

void UTranspersonalGameInstance::Shutdown()
{
	UE_LOG(LogTranspersonalGame, Warning, TEXT("TranspersonalGameInstance: Shutting down"));
	
	// Cleanup game systems
	CleanupGameSystems();
	
	Super::Shutdown();
}

void UTranspersonalGameInstance::InitializeStudioDirector()
{
	if (UStudioDirectorSubsystem* StudioDirector = GetSubsystem<UStudioDirectorSubsystem>())
	{
		UE_LOG(LogTranspersonalGame, Log, TEXT("Studio Director subsystem found and initialized"));
		
		// Initialize the 18-agent production pipeline
		StudioDirector->InitializeProductionPipeline();
	}
	else
	{
		UE_LOG(LogTranspersonalGame, Error, TEXT("Failed to get Studio Director subsystem"));
	}
}

void UTranspersonalGameInstance::SetupGameConfiguration()
{
	UE_LOG(LogTranspersonalGame, Log, TEXT("Setting up game configuration"));
	
	// Configure game settings based on B1 concept document
	SetupSurvivalGameSettings();
	SetupDinosaurBehaviorSettings();
	SetupWorldSettings();
}

void UTranspersonalGameInstance::SetupSurvivalGameSettings()
{
	// Configure survival mechanics
	SurvivalSettings.bEnableFear = true;
	SurvivalSettings.bEnableHunger = true;
	SurvivalSettings.bEnableThirst = true;
	SurvivalSettings.bEnableFatigue = true;
	
	// Player starts as prey, not predator
	SurvivalSettings.bPlayerIsPredator = false;
	SurvivalSettings.FearIntensityMultiplier = 1.5f;
	
	UE_LOG(LogTranspersonalGame, Log, TEXT("Survival settings configured - Player as prey"));
}

void UTranspersonalGameInstance::SetupDinosaurBehaviorSettings()
{
	// Configure dinosaur AI behavior
	DinosaurSettings.bEnableIndependentBehavior = true;
	DinosaurSettings.bEnableUniqueVariations = true;
	DinosaurSettings.bEnableDomestication = true;
	
	// Each dinosaur has unique physical traits
	DinosaurSettings.PhysicalVariationRange = 0.15f; // 15% variation
	DinosaurSettings.BehaviorVariationRange = 0.25f; // 25% variation
	
	UE_LOG(LogTranspersonalGame, Log, TEXT("Dinosaur behavior settings configured"));
}

void UTranspersonalGameInstance::SetupWorldSettings()
{
	// Configure world generation settings
	WorldSettings.BiomeType = EBiomeType::JurassicForest;
	WorldSettings.bEnableDynamicWeather = true;
	WorldSettings.bEnableDayNightCycle = true;
	WorldSettings.TimeScale = 1.0f; // Real-time initially
	
	UE_LOG(LogTranspersonalGame, Log, TEXT("World settings configured - Jurassic Forest biome"));
}

void UTranspersonalGameInstance::CleanupGameSystems()
{
	UE_LOG(LogTranspersonalGame, Log, TEXT("Cleaning up game systems"));
	
	// Cleanup will be handled by individual subsystems
	bIsGameInitialized = false;
}

void UTranspersonalGameInstance::SetGamePhase(EGamePhase NewPhase)
{
	if (CurrentGamePhase != NewPhase)
	{
		EGamePhase OldPhase = CurrentGamePhase;
		CurrentGamePhase = NewPhase;
		
		UE_LOG(LogTranspersonalGame, Log, TEXT("Game phase changed from %d to %d"), 
			   (int32)OldPhase, (int32)NewPhase);
		
		// Notify subsystems of phase change
		OnGamePhaseChanged(OldPhase, NewPhase);
	}
}

void UTranspersonalGameInstance::OnGamePhaseChanged(EGamePhase OldPhase, EGamePhase NewPhase)
{
	// Handle phase transition logic
	switch (NewPhase)
	{
	case EGamePhase::MainMenu:
		UE_LOG(LogTranspersonalGame, Log, TEXT("Entering Main Menu phase"));
		break;
		
	case EGamePhase::Loading:
		UE_LOG(LogTranspersonalGame, Log, TEXT("Entering Loading phase"));
		break;
		
	case EGamePhase::InGame:
		UE_LOG(LogTranspersonalGame, Log, TEXT("Entering In-Game phase"));
		break;
		
	case EGamePhase::Paused:
		UE_LOG(LogTranspersonalGame, Log, TEXT("Entering Paused phase"));
		break;
		
	default:
		UE_LOG(LogTranspersonalGame, Warning, TEXT("Unknown game phase: %d"), (int32)NewPhase);
		break;
	}
}