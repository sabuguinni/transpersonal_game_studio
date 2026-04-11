#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalGame, Log, All);

/**
 * Main module for the Transpersonal Game Studio prehistoric survival game.
 * Handles module initialization, consciousness system registration, and core game systems.
 */
class FTranspersonalGameModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    /** Check if the module is loaded and ready */
    static bool IsModuleLoaded();
    
    /** Get the module instance */
    static FTranspersonalGameModule& Get();

private:
    /** Initialize core game systems */
    void InitializeGameSystems();
    
    /** Initialize consciousness system */
    void InitializeConsciousnessSystem();
    
    /** Initialize prehistoric world systems */
    void InitializePrehistoricWorld();
    
    /** Initialize AI and behavior systems */
    void InitializeAISystems();
    
    /** Cleanup on shutdown */
    void CleanupGameSystems();
    
    /** Module loaded flag */
    static bool bModuleLoaded;
};