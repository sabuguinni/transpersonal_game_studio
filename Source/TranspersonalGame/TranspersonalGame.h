#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "Logging/LogMacros.h"

/**
 * TRANSPERSONAL GAME STUDIO - ENGINE ARCHITECTURE
 * 
 * Core module header defining the fundamental architecture principles
 * and system boundaries for the entire Transpersonal Game project.
 * 
 * ARCHITECTURAL PRINCIPLES:
 * 1. Modular Design - Each system is self-contained with clear interfaces
 * 2. Performance First - 60fps PC / 30fps Console targets are non-negotiable
 * 3. Scalable Systems - Support for 8km x 8km worlds with 50k+ AI agents
 * 4. Clean Dependencies - No circular dependencies, clear ownership hierarchy
 * 5. Future-Proof - Architecture supports years of feature additions
 */

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalGame, Log, All);

// Core system forward declarations
class UConsciousnessSystem;
class UPhysicsSystemManager;
class UWorldGenerationSystem;
class UPerformanceManager;
class UMassAISystem;

/**
 * Main module class for TranspersonalGame
 * Handles module lifecycle and core system initialization
 */
class FTranspersonalGameModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** Module state queries */
    static inline bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("TranspersonalGame"); }
    static inline FTranspersonalGameModule& Get() { return FModuleManager::LoadModuleChecked<FTranspersonalGameModule>("TranspersonalGame"); }

private:
    /** Initialize core systems in dependency order */
    void InitializeCoreSystem();
    
    /** Shutdown core systems in reverse dependency order */
    void ShutdownCoreSystems();

    /** Validate engine configuration for Transpersonal Game requirements */
    bool ValidateEngineConfiguration() const;

    /** Core system instances */
    TSharedPtr<UPerformanceManager> PerformanceManager;
};

/**
 * SYSTEM ARCHITECTURE OVERVIEW
 * 
 * LAYER 1 - ENGINE CORE (This Module)
 * - Module Management
 * - Performance Monitoring
 * - Memory Management
 * - Platform Abstraction
 * 
 * LAYER 2 - CORE SYSTEMS (Agent #3 responsibility)
 * - Physics System Manager
 * - Collision System
 * - Destruction System
 * - Ragdoll System
 * 
 * LAYER 3 - WORLD SYSTEMS (Agents #5-8)
 * - Procedural World Generation
 * - Environment Population
 * - Architecture System
 * - Lighting & Atmosphere
 * 
 * LAYER 4 - CHARACTER SYSTEMS (Agents #9-13)
 * - Character Creation
 * - Animation System
 * - NPC Behavior
 * - Combat AI
 * - Mass Simulation
 * 
 * LAYER 5 - GAMEPLAY SYSTEMS (Agents #14-17)
 * - Quest System
 * - Narrative System
 * - Audio System
 * - VFX System
 */

// Global constants defining system limits and targets
namespace TranspersonalConstants
{
    // Performance targets
    constexpr float TARGET_FPS_PC = 60.0f;
    constexpr float TARGET_FPS_CONSOLE = 30.0f;
    constexpr float MAX_FRAME_TIME_MS = 16.67f; // 60fps
    
    // World limits
    constexpr float MAX_WORLD_SIZE_CM = 819200.0f; // 8.192km in UE5 units
    constexpr int32 MAX_STREAMING_LEVELS = 64;
    constexpr int32 WORLD_PARTITION_GRID_SIZE = 512; // 512m grid cells
    
    // AI and simulation limits
    constexpr int32 MAX_MASS_AGENTS = 50000;
    constexpr int32 MAX_ACTIVE_AI = 200;
    constexpr float AI_LOD_DISTANCE_NEAR = 5000.0f; // 50m
    constexpr float AI_LOD_DISTANCE_FAR = 20000.0f;  // 200m
    
    // Memory budgets (in MB)
    constexpr int32 TEXTURE_MEMORY_BUDGET_PC = 4096;
    constexpr int32 TEXTURE_MEMORY_BUDGET_CONSOLE = 2048;
    constexpr int32 MESH_MEMORY_BUDGET = 1024;
    constexpr int32 AUDIO_MEMORY_BUDGET = 512;
}

// Utility macros for consistent logging and debugging
#define TRANSPERSONAL_LOG(Verbosity, Format, ...) \
    UE_LOG(LogTranspersonalGame, Verbosity, Format, ##__VA_ARGS__)

#define TRANSPERSONAL_LOG_WARNING(Format, ...) \
    TRANSPERSONAL_LOG(Warning, Format, ##__VA_ARGS__)

#define TRANSPERSONAL_LOG_ERROR(Format, ...) \
    TRANSPERSONAL_LOG(Error, Format, ##__VA_ARGS__)

// Performance profiling macros (only in development builds)
#if TRANSPERSONAL_DEVELOPMENT
    #define TRANSPERSONAL_SCOPE_CYCLE_COUNTER(StatId) SCOPE_CYCLE_COUNTER(StatId)
    #define TRANSPERSONAL_DECLARE_CYCLE_STAT(StatName, GroupName) DECLARE_CYCLE_STAT(StatName, STAT_##StatName, STATGROUP_##GroupName)
#else
    #define TRANSPERSONAL_SCOPE_CYCLE_COUNTER(StatId)
    #define TRANSPERSONAL_DECLARE_CYCLE_STAT(StatName, GroupName)
#endif