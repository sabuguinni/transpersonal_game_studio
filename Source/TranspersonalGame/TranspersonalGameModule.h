#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalGame, Log, All);

/**
 * Main module class for the Transpersonal Game
 * Handles initialization and coordination of all game systems
 * 
 * This module serves as the entry point for the entire game,
 * coordinating between the 18 specialized agent systems.
 */
class TRANSPERSONALGAME_API FTranspersonalGameModule : public IModuleInterface
{
public:
	/**
	 * Called when the module is loaded into memory
	 * Initializes core subsystems and registers components
	 */
	virtual void StartupModule() override;
	
	/**
	 * Called when the module is unloaded from memory
	 * Cleans up subsystems and unregisters components
	 */
	virtual void ShutdownModule() override;

private:
	/**
	 * Initialize core subsystems required by the game
	 * This includes the Studio Director subsystem and performance monitoring
	 */
	void InitializeCoreSubsystems();
	
	/**
	 * Register game-specific components with the engine
	 * Called during module startup to ensure proper initialization order
	 */
	void RegisterGameComponents();
	
	/**
	 * Cleanup all subsystems during shutdown
	 * Ensures proper shutdown order and resource cleanup
	 */
	void CleanupSubsystems();
};