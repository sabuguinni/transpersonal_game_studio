#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Core_PhysicsGameModeIntegrator.generated.h"

// Forward declarations
class UCore_PhysicsManager;
class UCore_CollisionSystem;
class UCore_RagdollSystem;
class UCore_DestructionSystem;
class UCore_PhysicsWorldManager;
class AEng_GameModeArchitect;

/**
 * Core_PhysicsGameModeIntegrator
 * 
 * Integrates all Core Physics Systems with the Engine Architect's GameMode framework.
 * This class serves as the bridge between the GameMode architecture and the physics simulation layer.
 * 
 * INTEGRATION PHILOSOPHY:
 * - Physics systems register with GameMode during initialization phase
 * - GameMode coordinates physics system startup order and dependencies
 * - Performance monitoring flows through GameMode's metrics collection
 * - Error handling and validation uses GameMode's framework
 * 
 * SYSTEM COORDINATION:
 * 1. Physics Manager → Core physics simulation control
 * 2. Collision System → Collision detection and response
 * 3. Ragdoll System → Character physics and death states
 * 4. Destruction System → Environmental destruction physics
 * 5. World Manager → Physics world streaming and LOD
 */
UCLASS(BlueprintType, Blueprintable, Category = "Core Systems|Physics Integration")
class TRANSPERSONALGAME_API UCore_PhysicsGameModeIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsGameModeIntegrator();

    // === GAMEMODE INTEGRATION INTERFACE ===
    
    /**
     * Initialize physics systems integration with GameMode architecture
     * Called by GameMode during system initialization phase
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void InitializePhysicsIntegration(AEng_GameModeArchitect* GameModeArchitect);
    
    /**
     * Register all physics subsystems with the GameMode framework
     * Ensures proper initialization order and dependency management
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool RegisterPhysicsSubsystems();
    
    /**
     * Validate physics system integration and dependencies
     * Returns comprehensive status report for GameMode validation
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool ValidatePhysicsIntegration();

    // === PHYSICS SYSTEM COORDINATION ===
    
    /**
     * Configure physics settings for Cretaceous period realistic simulation
     * Sets up gravity, material properties, and environmental physics
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Configuration")
    void ConfigureCretaceousPhysics();
    
    /**
     * Initialize dinosaur physics systems
     * Sets up ragdoll, collision, and movement physics for prehistoric creatures
     */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void InitializeDinosaurPhysics();
    
    /**
     * Configure environmental destruction physics
     * Sets up tree falling, rock breaking, and terrain deformation
     */
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ConfigureEnvironmentalDestruction();

    // === PERFORMANCE INTEGRATION ===
    
    /**
     * Collect physics performance metrics for GameMode monitoring
     * Integrates with GameMode's performance dashboard
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void CollectPhysicsMetrics();
    
    /**
     * Get current physics performance status
     * Returns metrics formatted for GameMode's monitoring system
     */
    UFUNCTION(BlueprintPure, Category = "Performance Monitoring")
    FString GetPhysicsPerformanceStatus() const;

    // === WORLD INTEGRATION ===
    
    /**
     * Handle world streaming events for physics systems
     * Coordinates with World Partition for physics LOD management
     */
    UFUNCTION(BlueprintCallable, Category = "World Integration")
    void HandleWorldStreamingEvent(bool bIsLoading, const FString& StreamingLevel);
    
    /**
     * Update physics systems for current world state
     * Called by GameMode during world updates
     */
    UFUNCTION(BlueprintCallable, Category = "World Integration")
    void UpdatePhysicsForWorld();

protected:
    // === COMPONENT LIFECYCLE ===
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === PHYSICS SYSTEM REFERENCES ===
    
    /** Reference to the main physics manager */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsManager> PhysicsManager;
    
    /** Reference to the collision system */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_CollisionSystem> CollisionSystem;
    
    /** Reference to the ragdoll system */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_RagdollSystem> RagdollSystem;
    
    /** Reference to the destruction system */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_DestructionSystem> DestructionSystem;
    
    /** Reference to the physics world manager */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsWorldManager> PhysicsWorldManager;

    // === GAMEMODE INTEGRATION ===
    
    /** Reference to the GameMode architect for system coordination */
    UPROPERTY(BlueprintReadOnly, Category = "GameMode Integration", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AEng_GameModeArchitect> GameModeArchitectRef;
    
    /** Integration status tracking */
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status", meta = (AllowPrivateAccess = "true"))
    bool bIsIntegrationInitialized;
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsSystemsRegistered;
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration Status", meta = (AllowPrivateAccess = "true"))
    bool bValidationPassed;

    // === PHYSICS CONFIGURATION ===
    
    /** Cretaceous period gravity setting (Earth gravity ~66M years ago) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Configuration", meta = (AllowPrivateAccess = "true"))
    float CretaceousGravity = -980.0f;
    
    /** Air density for Cretaceous atmosphere (higher oxygen content) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Configuration", meta = (AllowPrivateAccess = "true"))
    float CretaceousAirDensity = 1.35f;
    
    /** Enable realistic dinosaur physics simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics", meta = (AllowPrivateAccess = "true"))
    bool bEnableRealisticDinosaurPhysics = true;
    
    /** Enable environmental destruction physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics", meta = (AllowPrivateAccess = "true"))
    bool bEnableEnvironmentalDestruction = true;

    // === PERFORMANCE TRACKING ===
    
    /** Current physics performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentPhysicsFrameTime;
    
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 ActivePhysicsActors;
    
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 ActiveRagdollCount;
    
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float PhysicsMemoryUsageMB;

private:
    // === INTERNAL INTEGRATION HELPERS ===
    
    /** Initialize physics system references */
    void InitializePhysicsSystemReferences();
    
    /** Validate all physics system dependencies */
    bool ValidatePhysicsSystemDependencies();
    
    /** Configure physics settings for realistic simulation */
    void ApplyRealisticPhysicsSettings();
    
    /** Set up physics collision channels for dinosaur game */
    void ConfigureCollisionChannels();
    
    /** Initialize physics materials for different surfaces */
    void InitializePhysicsMaterials();
    
    /** Update performance metrics */
    void UpdatePerformanceMetrics();
    
    /** Handle physics system errors */
    void HandlePhysicsSystemError(const FString& SystemName, const FString& ErrorMessage);

    // === INTEGRATION STATE ===
    
    /** Timer for performance metric updates */
    FTimerHandle PerformanceUpdateTimer;
    
    /** Last validation timestamp */
    double LastValidationTime;
    
    /** Integration initialization timestamp */
    double IntegrationStartTime;
};