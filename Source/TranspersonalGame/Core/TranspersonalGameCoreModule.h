// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalGameCore, Log, All);

/**
 * Core Module for Transpersonal Game
 * 
 * Manages initialization and coordination of core game systems:
 * - Studio Director coordination
 * - Physics system management
 * - Performance monitoring
 * - Core game framework components
 */
class TRANSPERSONALGAME_API FTranspersonalGameCoreModule : public IModuleInterface
{
public:
    // IModuleInterface implementation
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    /** Initialize core game systems */
    void InitializeCoreSystems();
    
    /** Shutdown core game systems */
    void ShutdownCoreSystems();
    
    /** Register this module with the Studio Director */
    void RegisterWithStudioDirector();
};