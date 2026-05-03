#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Core_PhysicsSystemManager.generated.h"

// Forward declarations
class UCore_TerrainInteractionSystem;
class UCore_SurvivalSystem;
class ATranspersonalCharacter;

/**
 * Core Physics System Manager
 * Manages all physics-related systems in the game including terrain interaction,
 * survival mechanics, and character physics integration.
 * 
 * This is the central hub for all physics calculations and ensures proper
 * coordination between different physics subsystems.
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * Initialize all physics systems
     * Called during BeginPlay to set up all subsystems
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSystems();

    /**
     * Update all physics systems
     * Called every tick to update physics calculations
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePhysicsSystems(float DeltaTime);

    /**
     * Register a character with the physics system
     * Allows the physics system to track and manage character physics
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void RegisterCharacter(ATranspersonalCharacter* Character);

    /**
     * Unregister a character from the physics system
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UnregisterCharacter(ATranspersonalCharacter* Character);

    /**
     * Get the terrain interaction system
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    UCore_TerrainInteractionSystem* GetTerrainInteractionSystem() const { return TerrainInteractionSystem; }

    /**
     * Get the survival system
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    UCore_SurvivalSystem* GetSurvivalSystem() const { return SurvivalSystem; }

    /**
     * Check if physics systems are initialized
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool ArePhysicsSystemsInitialized() const { return bPhysicsSystemsInitialized; }

    /**
     * Enable/disable physics system updates
     */
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetPhysicsSystemsEnabled(bool bEnabled) { bPhysicsSystemsEnabled = bEnabled; }

protected:
    /** Terrain interaction system component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    UCore_TerrainInteractionSystem* TerrainInteractionSystem;

    /** Survival system component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    UCore_SurvivalSystem* SurvivalSystem;

    /** List of registered characters */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    TArray<ATranspersonalCharacter*> RegisteredCharacters;

    /** Whether physics systems are initialized */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsSystemsInitialized;

    /** Whether physics systems are enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsSystemsEnabled;

    /** Physics update frequency (times per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Systems", meta = (AllowPrivateAccess = "true"))
    float PhysicsUpdateFrequency;

    /** Time accumulator for physics updates */
    float PhysicsTimeAccumulator;

private:
    /**
     * Create and initialize subsystem components
     */
    void CreateSubsystems();

    /**
     * Validate that all required systems are present
     */
    bool ValidatePhysicsSystems() const;
};