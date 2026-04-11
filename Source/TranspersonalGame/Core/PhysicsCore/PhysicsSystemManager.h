#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsSystemManager.generated.h"

class UAdvancedRagdollComponent;
class UDestructionComponent;
class UVehiclePhysicsComponent;

/**
 * @brief Central manager for all physics systems in the Transpersonal Game
 * 
 * This class orchestrates physics simulation, manages performance budgets,
 * and provides unified interface for all physics-related operations.
 * 
 * Key responsibilities:
 * - Physics simulation management and optimization
 * - Ragdoll system coordination
 * - Destruction system oversight
 * - Vehicle physics integration
 * - Performance monitoring and LOD management
 * 
 * @author Core Systems Programmer - Agent #03
 * @version 1.0
 * @date 2024
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Initialize the physics system with performance parameters
     * @param MaxSimulatedBodies Maximum number of physics bodies to simulate simultaneously
     * @param PhysicsLODDistance Distance at which physics LOD kicks in
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem(int32 MaxSimulatedBodies = 1000, float PhysicsLODDistance = 5000.0f);

    /**
     * @brief Register a ragdoll component for management
     * @param RagdollComponent The ragdoll component to register
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterRagdollComponent(UAdvancedRagdollComponent* RagdollComponent);

    /**
     * @brief Register a destruction component for management
     * @param DestructionComponent The destruction component to register
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterDestructionComponent(UDestructionComponent* DestructionComponent);

    /**
     * @brief Register a vehicle physics component for management
     * @param VehicleComponent The vehicle physics component to register
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterVehicleComponent(UVehiclePhysicsComponent* VehicleComponent);

    /**
     * @brief Get current physics performance metrics
     * @return Struct containing performance data
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FPhysicsPerformanceData GetPhysicsPerformanceData() const;

    /**
     * @brief Force physics LOD update for all managed components
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UpdatePhysicsLOD();

    /**
     * @brief Enable/disable physics simulation globally
     * @param bEnabled Whether physics should be enabled
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsEnabled(bool bEnabled);

protected:
    /** Maximum number of physics bodies that can be simulated simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimulatedBodies;

    /** Distance at which physics LOD reduction begins */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsLODDistance;

    /** Whether physics simulation is currently enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System")
    bool bPhysicsEnabled;

    /** Current frame time budget for physics (in milliseconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsTimeBudget;

private:
    /** Registered ragdoll components */
    UPROPERTY()
    TArray<UAdvancedRagdollComponent*> RegisteredRagdolls;

    /** Registered destruction components */
    UPROPERTY()
    TArray<UDestructionComponent*> RegisteredDestructionComponents;

    /** Registered vehicle physics components */
    UPROPERTY()
    TArray<UVehiclePhysicsComponent*> RegisteredVehicleComponents;

    /** Current physics performance metrics */
    mutable FPhysicsPerformanceData CachedPerformanceData;

    /** Last time performance data was updated */
    mutable float LastPerformanceUpdateTime;

    /** Performance data cache duration */
    static constexpr float PERFORMANCE_CACHE_DURATION = 0.1f; // 100ms

    /**
     * @brief Update internal performance metrics
     */
    void UpdatePerformanceMetrics() const;

    /**
     * @brief Apply LOD to physics components based on distance
     */
    void ApplyPhysicsLOD();

    /**
     * @brief Manage physics simulation budget
     */
    void ManagePhysicsBudget(float DeltaTime);
};

/**
 * @brief Structure containing physics performance data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPhysicsPerformanceData
{
    GENERATED_BODY()

    /** Number of currently active physics bodies */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsBodies;

    /** Current physics simulation time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsSimulationTime;

    /** Number of active ragdoll instances */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRagdolls;

    /** Number of active destruction instances */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDestructions;

    /** Number of active vehicle physics instances */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveVehicles;

    /** Current physics LOD level (0 = full detail, higher = reduced detail) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentLODLevel;

    FPhysicsPerformanceData()
        : ActivePhysicsBodies(0)
        , PhysicsSimulationTime(0.0f)
        , ActiveRagdolls(0)
        , ActiveDestructions(0)
        , ActiveVehicles(0)
        , CurrentLODLevel(0)
    {}
};