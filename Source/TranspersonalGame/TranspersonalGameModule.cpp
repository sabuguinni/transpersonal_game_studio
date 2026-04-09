#include "TranspersonalGameModule.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY(LogTranspersonalGame);

IMPLEMENT_PRIMARY_GAME_MODULE(FTranspersonalGameModule, TranspersonalGame, "TranspersonalGame");

void FTranspersonalGameModule::StartupModule()
{
	UE_LOG(LogTranspersonalGame, Warning, TEXT("TranspersonalGame Module: Startup"));
	
	// Initialize core subsystems
	InitializeCoreSubsystems();
	
	// Register game-specific components
	RegisterGameComponents();
	
	UE_LOG(LogTranspersonalGame, Warning, TEXT("TranspersonalGame Module: Startup Complete"));
}

void FTranspersonalGameModule::ShutdownModule()
{
	UE_LOG(LogTranspersonalGame, Warning, TEXT("TranspersonalGame Module: Shutdown"));
	
	// Cleanup subsystems
	CleanupSubsystems();
	
	UE_LOG(LogTranspersonalGame, Warning, TEXT("TranspersonalGame Module: Shutdown Complete"));
}

void FTranspersonalGameModule::InitializeCoreSubsystems()
{
	// Studio Director subsystem will be auto-initialized by UE5
	UE_LOG(LogTranspersonalGame, Log, TEXT("Initializing Core Subsystems"));
	
	// Register performance monitoring
	if (GEngine)
	{
		UE_LOG(LogTranspersonalGame, Log, TEXT("Engine available - Core systems ready"));
	}
}

void FTranspersonalGameModule::RegisterGameComponents()
{
	UE_LOG(LogTranspersonalGame, Log, TEXT("Registering Game Components"));
	
	// Component registration will be handled by individual modules
	// This is called during module startup to ensure proper initialization order
}

void FTranspersonalGameModule::CleanupSubsystems()
{
	UE_LOG(LogTranspersonalGame, Log, TEXT("Cleaning up subsystems"));
	
	// Cleanup will be handled by individual subsystems
	// This ensures proper shutdown order
}