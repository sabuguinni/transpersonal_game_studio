#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Core_PhysicsIntegration.generated.h"

class UCore_PhysicsCore;
class UEng_SystemRegistry;

/**
 * Core Physics Integration Subsystem
 * Integrates physics systems with SystemRegistry and provides centralized physics management
 * for survival gameplay mechanics including terrain interaction, character physics, and dinosaur behavior
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsIntegration : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsIntegration();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Initialize physics systems through SystemRegistry
     * Called by SystemRegistry during controlled initialization
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    bool InitializePhysicsSystems();

    /**
     * Shutdown physics systems
     * Called by SystemRegistry during controlled shutdown
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    void ShutdownPhysicsSystems();

    /**
     * Get physics system status
     * Returns current initialization and health status
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    ECore_SystemStatus GetPhysicsSystemStatus() const;

    /**
     * Register physics system with SystemRegistry
     * Ensures proper lifecycle management
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    bool RegisterWithSystemRegistry();

    /**
     * Apply survival physics profile to character
     * Configures physics for realistic survival gameplay
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    void ApplySurvivalPhysicsProfile(AActor* Character);

    /**
     * Apply dinosaur physics profile
     * Configures physics for dinosaur AI and behavior
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    void ApplyDinosaurPhysicsProfile(AActor* Dinosaur, ECore_DinosaurSize DinosaurSize);

    /**
     * Configure terrain physics interaction
     * Sets up terrain-specific physics responses
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    void ConfigureTerrainPhysics(AActor* TerrainActor, ECore_TerrainType TerrainType);

    /**
     * Enable ragdoll physics for survival scenarios
     * Handles character death, unconsciousness, and impact physics
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    void EnableSurvivalRagdoll(AActor* Character, float ImpactForce = 1000.0f);

    /**
     * Configure projectile physics for primitive weapons
     * Handles spears, rocks, and other thrown weapons
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    void ConfigureProjectilePhysics(AActor* Projectile, float Mass = 0.5f, float Drag = 0.1f);

    /**
     * Update physics simulation quality based on performance
     * Dynamically adjusts physics quality for optimal performance
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    void UpdatePhysicsQuality(float DeltaTime);

    /**
     * Get physics performance metrics
     * Returns current physics system performance data
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    FCore_PhysicsMetrics GetPhysicsMetrics() const;

    /**
     * Validate physics system integrity
     * Checks for physics system errors and inconsistencies
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics Integration")
    bool ValidatePhysicsIntegrity();

protected:
    /** Reference to core physics system */
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics Integration")
    UCore_PhysicsCore* PhysicsCore;

    /** Reference to system registry */
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics Integration")
    UEng_SystemRegistry* SystemRegistry;

    /** Current physics system status */
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics Integration")
    ECore_SystemStatus PhysicsStatus;

    /** Physics performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics Integration")
    FCore_PhysicsMetrics CurrentMetrics;

    /** Physics quality level (0.0 = lowest, 1.0 = highest) */
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics Integration", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PhysicsQualityLevel;

    /** Time since last physics update */
    UPROPERTY(BlueprintReadOnly, Category = "Core Physics Integration")
    float TimeSinceLastUpdate;

    /** Physics update frequency in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics Integration", meta = (ClampMin = "0.016", ClampMax = "1.0"))
    float PhysicsUpdateFrequency;

private:
    /** Internal initialization flag */
    bool bIsInitialized;

    /** Internal shutdown flag */
    bool bIsShuttingDown;

    /** Performance monitoring timer */
    float PerformanceTimer;

    /** Last frame physics time */
    float LastPhysicsTime;

    /**
     * Internal method to setup physics profiles
     * Configures default physics profiles for different object types
     */
    void SetupPhysicsProfiles();

    /**
     * Internal method to validate system dependencies
     * Ensures all required systems are available
     */
    bool ValidateSystemDependencies() const;

    /**
     * Internal method to update performance metrics
     * Calculates and updates physics performance data
     */
    void UpdatePerformanceMetrics(float DeltaTime);
};