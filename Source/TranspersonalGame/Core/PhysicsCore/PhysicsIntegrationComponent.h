// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsIntegrationComponent.generated.h"

// Forward declarations
class UChaosPhysicsManager;
class UCollisionSystem;
class UPhysicsSystemManager;

/**
 * Physics Integration Component
 * 
 * Provides seamless integration between different physics systems
 * in the Transpersonal Game. Handles communication between:
 * - Chaos Physics Manager
 * - Collision System
 * - Physics System Manager
 * - Destruction System
 * - Ragdoll System
 * 
 * This component acts as a central hub for physics-related operations
 * and ensures consistent behavior across all physics subsystems.
 * 
 * @author Core Systems Programmer — Agent #3
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsIntegrationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsIntegrationComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * Initialize physics integration for this actor
     * Sets up connections to all relevant physics subsystems
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void InitializePhysicsIntegration();

    /**
     * Register this actor with physics systems
     * @param PhysicsType Type of physics simulation (Creature, Environment, Debris, etc.)
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void RegisterWithPhysicsSystems(const FString& PhysicsType);

    /**
     * Unregister from all physics systems
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UnregisterFromPhysicsSystems();

    /**
     * Enable/disable physics simulation for this actor
     * @param bEnabled Whether to enable physics simulation
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetPhysicsEnabled(bool bEnabled);

    /**
     * Apply physics profile to this actor
     * @param ProfileName Name of the physics profile to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyPhysicsProfile(const FString& ProfileName);

    /**
     * Get current physics status
     * @return True if physics is currently enabled and active
     */
    UFUNCTION(BlueprintPure, Category = "Physics Integration")
    bool IsPhysicsActive() const;

    /**
     * Force physics update for this actor
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ForcePhysicsUpdate();

    /**
     * Set physics LOD level
     * @param LODLevel Level of detail (0 = highest, 3 = lowest)
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetPhysicsLOD(int32 LODLevel);

    /**
     * Get distance to nearest player for LOD calculations
     * @return Distance in units to the nearest player
     */
    UFUNCTION(BlueprintPure, Category = "Physics Integration")
    float GetDistanceToNearestPlayer() const;

protected:
    /** Type of physics simulation for this actor */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Integration")
    FString PhysicsType = "Default";

    /** Whether physics is currently enabled */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    bool bPhysicsEnabled = true;

    /** Current physics LOD level */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    int32 CurrentPhysicsLOD = 0;

    /** Physics profile name applied to this actor */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Integration")
    FString AppliedPhysicsProfile = "Default";

    /** Whether this component is registered with physics systems */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    bool bRegisteredWithPhysics = false;

    /** Update frequency for LOD calculations (in seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float LODUpdateFrequency = 1.0f;

private:
    /** Update physics LOD based on distance to player */
    void UpdatePhysicsLOD();

    /** Get appropriate LOD level for given distance */
    int32 GetLODLevelForDistance(float Distance) const;

    /** Cached references to physics subsystems */
    UPROPERTY()
    UChaosPhysicsManager* ChaosPhysicsManager = nullptr;

    UPROPERTY()
    UCollisionSystem* CollisionSystem = nullptr;

    UPROPERTY()
    UPhysicsSystemManager* PhysicsSystemManager = nullptr;

    /** Timer handle for LOD updates */
    FTimerHandle LODUpdateTimer;

    /** LOD distance thresholds */
    static const TArray<float> LODDistances;
};