#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_CoreArchitecture.h"
#include "Core_PhysicsManager.h"
#include "Core_CollisionSystem.h"
#include "Core_RagdollSystem.h"
#include "Core_DestructionSystem.h"
#include "Core_TerrainPhysics.h"
#include "Core_MaterialPhysics.h"
#include "Core_FluidDynamics.h"
#include "Core_PhysicsArchitectureAdapter.generated.h"

/**
 * Core Physics Architecture Adapter
 * 
 * Integrates all physics systems with the Core Architecture Registry.
 * Implements the IEng_ArchitecturalSystem interface for physics domain.
 * Manages initialization order, dependencies, and health monitoring for all physics systems.
 * 
 * This adapter ensures that:
 * - All physics systems follow architectural standards
 * - Initialization happens in correct dependency order
 * - Performance metrics are tracked and reported
 * - System health is monitored continuously
 * - Integration with other game systems is managed properly
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsArchitectureAdapter : public UGameInstanceSubsystem, public IEng_ArchitecturalSystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsArchitectureAdapter();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // IEng_ArchitecturalSystem interface
    virtual FString GetSystemName() const override;
    virtual TArray<FString> GetDependencies() const override;
    virtual bool InitializeSystem() override;
    virtual void ShutdownSystem() override;
    virtual bool ValidateSystem() const override;
    virtual FEng_SystemHealthReport GetHealthReport() const override;
    virtual void HandleArchitecturalEvent(const FEng_ArchitecturalEvent& Event) override;

    // Physics Systems Management
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool InitializePhysicsSubsystems();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void ShutdownPhysicsSubsystems();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool ValidatePhysicsIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    FString GetPhysicsSystemsStatus() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPhysicsFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    int32 GetActivePhysicsObjects() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPhysicsMemoryUsage() const;

    // System Health
    UFUNCTION(BlueprintCallable, Category = "Physics Health")
    bool IsPhysicsSystemHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Health")
    TArray<FString> GetPhysicsWarnings() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Health")
    TArray<FString> GetPhysicsErrors() const;

protected:
    // Physics System References
    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_PhysicsManager> PhysicsManager;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_CollisionSystem> CollisionSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_RagdollSystem> RagdollSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_DestructionSystem> DestructionSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_TerrainPhysics> TerrainPhysics;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_MaterialPhysics> MaterialPhysics;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Systems")
    TObjectPtr<UCore_FluidDynamics> FluidDynamics;

    // Architecture Integration
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TObjectPtr<UEng_CoreArchitecture> CoreArchitecture;

    // System State
    UPROPERTY(BlueprintReadOnly, Category = "System State")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "System State")
    bool bIsHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "System State")
    float LastHealthCheckTime;

    // Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PeakPhysicsObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PeakMemoryUsage;

    // Health Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Health")
    TArray<FString> ActiveWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    TArray<FString> ActiveErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 ConsecutiveHealthyFrames;

private:
    // Internal System Management
    bool RegisterWithCoreArchitecture();
    void UnregisterFromCoreArchitecture();
    
    bool InitializePhysicsManager();
    bool InitializeCollisionSystem();
    bool InitializeRagdollSystem();
    bool InitializeDestructionSystem();
    bool InitializeTerrainPhysics();
    bool InitializeMaterialPhysics();
    bool InitializeFluidDynamics();

    // Health Monitoring
    void UpdateHealthMetrics();
    void CheckSystemIntegrity();
    void UpdatePerformanceMetrics();
    
    // Event Handling
    void OnPhysicsSystemEvent(const FString& SystemName, const FString& EventType, const FString& EventData);
    void OnArchitectureValidationRequired();
    void OnPerformanceThresholdExceeded(const FString& MetricName, float Value, float Threshold);

    // Constants
    static constexpr float HEALTH_CHECK_INTERVAL = 1.0f;
    static constexpr float PERFORMANCE_WARNING_THRESHOLD = 16.67f; // 60 FPS
    static constexpr float MEMORY_WARNING_THRESHOLD = 512.0f; // 512 MB
    static constexpr int32 MAX_PHYSICS_OBJECTS_WARNING = 10000;
};