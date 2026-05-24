#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalGame, Log, All);

/**
 * Main module for Transpersonal Game Studio's prehistoric consciousness simulation
 * Handles initialization of all core systems and subsystems
 */
class TRANSPERSONALGAME_API FTranspersonalGameModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    /** Check if module is loaded and ready */
    static bool IsModuleLoaded();
    
    /** Get module instance */
    static FTranspersonalGameModule& Get();
    
    /** Module initialization state */
    bool IsInitialized() const { return bIsInitialized; }
    
private:
    /** Initialize core systems */
    void InitializeCoreSystem();
    
    /** Initialize consciousness simulation */
    void InitializeConsciousnessSystem();
    
    /** Initialize world systems */
    void InitializeWorldSystems();
    
    /** Initialize AI systems */
    void InitializeAISystems();
    
    /** Cleanup all systems */
    void CleanupSystems();
    
    /** Module state */
    bool bIsInitialized = false;
    
    /** System managers */
    TSharedPtr<class FConsciousnessSystemManager> ConsciousnessManager;
    TSharedPtr<class FWorldSystemManager> WorldManager;
    TSharedPtr<class FAISystemManager> AIManager;
};