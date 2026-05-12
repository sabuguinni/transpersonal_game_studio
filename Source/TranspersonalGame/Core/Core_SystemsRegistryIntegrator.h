#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_SystemsRegistry.h"
#include "SharedTypes.h"
#include "Core_SystemsRegistryIntegrator.generated.h"

// Forward declarations
class UCore_PhysicsManager;
class UCore_CollisionSystem;
class UCore_RagdollSystem;
class UCore_DestructionSystem;
class UCore_MaterialPhysics;

/**
 * Core Systems Registry Integrator
 * 
 * Integrates all Core Systems (Physics, Collision, Ragdoll, Destruction, Materials)
 * with the Engine Architect's Systems Registry. Manages registration, initialization
 * order, and dependency resolution for all physics-related systems.
 * 
 * This class ensures that physics systems are registered with proper priorities:
 * - Core_PhysicsManager: Priority 10 (Foundation)
 * - Core_CollisionSystem: Priority 11 (Collision Detection)
 * - Core_MaterialPhysics: Priority 12 (Material Properties)
 * - Core_RagdollSystem: Priority 13 (Character Physics)
 * - Core_DestructionSystem: Priority 14 (Destruction Physics)
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_SystemsRegistryIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_SystemsRegistryIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Register all core physics systems with the Systems Registry
     * Called during subsystem initialization
     */
    UFUNCTION(BlueprintCallable, Category = "Core Systems")
    void RegisterPhysicsSystems();

    /**
     * Initialize all registered physics systems in dependency order
     * Called by Systems Registry when dependencies are satisfied
     */
    UFUNCTION(BlueprintCallable, Category = "Core Systems")
    void InitializePhysicsSystems();

    /**
     * Validate that all physics systems are properly integrated
     * Returns true if all systems are registered and functional
     */
    UFUNCTION(BlueprintCallable, Category = "Core Systems")
    bool ValidatePhysicsSystemsIntegration();

    /**
     * Get the physics system registration status
     * Returns detailed information about each system's state
     */
    UFUNCTION(BlueprintCallable, Category = "Core Systems")
    FString GetPhysicsSystemsStatus();

    /**
     * Emergency shutdown of all physics systems
     * Used when critical errors are detected
     */
    UFUNCTION(BlueprintCallable, Category = "Core Systems")
    void EmergencyShutdownPhysicsSystems();

protected:
    /**
     * Reference to the Engine Architect's Systems Registry
     * Used for registering and coordinating with other systems
     */
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TObjectPtr<UEng_SystemsRegistry> SystemsRegistry;

    /**
     * Core Physics Manager - Foundation physics system
     * Priority 10 - Must initialize before all other physics systems
     */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_PhysicsManager> PhysicsManager;

    /**
     * Collision System - Handles collision detection and response
     * Priority 11 - Depends on PhysicsManager
     */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_CollisionSystem> CollisionSystem;

    /**
     * Material Physics - Manages material properties and interactions
     * Priority 12 - Depends on PhysicsManager and CollisionSystem
     */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_MaterialPhysics> MaterialPhysics;

    /**
     * Ragdoll System - Character physics and ragdoll simulation
     * Priority 13 - Depends on all previous physics systems
     */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_RagdollSystem> RagdollSystem;

    /**
     * Destruction System - Handles object destruction and debris
     * Priority 14 - Depends on all previous physics systems
     */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_DestructionSystem> DestructionSystem;

    /**
     * Track registration status of each system
     * Used for validation and debugging
     */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TMap<FString, bool> SystemRegistrationStatus;

    /**
     * Track initialization status of each system
     * Used for dependency resolution and error handling
     */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TMap<FString, bool> SystemInitializationStatus;

    /**
     * Performance metrics for system initialization
     * Used for optimization and monitoring
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TMap<FString, float> SystemInitializationTimes;

private:
    /**
     * Create and configure the physics manager
     * Internal helper for system setup
     */
    void CreatePhysicsManager();

    /**
     * Create and configure the collision system
     * Internal helper for system setup
     */
    void CreateCollisionSystem();

    /**
     * Create and configure the material physics system
     * Internal helper for system setup
     */
    void CreateMaterialPhysics();

    /**
     * Create and configure the ragdoll system
     * Internal helper for system setup
     */
    void CreateRagdollSystem();

    /**
     * Create and configure the destruction system
     * Internal helper for system setup
     */
    void CreateDestructionSystem();

    /**
     * Validate system dependencies before initialization
     * Ensures systems are created in correct order
     */
    bool ValidateSystemDependencies();

    /**
     * Log system registration and initialization events
     * Used for debugging and monitoring
     */
    void LogSystemEvent(const FString& SystemName, const FString& Event, bool bSuccess);
};