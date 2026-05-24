#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Core_PhysicsArchitecturalIntegrator.generated.h"

// Forward declarations
class UEng_ArchitecturalCore;
class UCore_PhysicsManager;
class UCore_CollisionSystem;
class UCore_RagdollSystem;
class UCore_DestructionSystem;

/**
 * Data structure for physics system registration with architectural core
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSystemRegistration
{
    GENERATED_BODY()

    /** System name for identification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Registration")
    FString SystemName;

    /** Priority for initialization order (lower = earlier) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Registration")
    int32 InitializationPriority;

    /** Whether system is critical for gameplay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Registration")
    bool bIsCriticalSystem;

    /** System dependencies (must be initialized first) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Registration")
    TArray<FString> Dependencies;

    /** Performance budget in milliseconds per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Registration")
    float PerformanceBudgetMs;

    /** System status for monitoring */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Registration")
    ESystemStatus SystemStatus;

    FCore_PhysicsSystemRegistration()
    {
        SystemName = TEXT("");
        InitializationPriority = 100;
        bIsCriticalSystem = false;
        PerformanceBudgetMs = 1.0f;
        SystemStatus = ESystemStatus::Uninitialized;
    }
};

/**
 * Physics system performance metrics for architectural monitoring
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    /** Average frame time for physics calculations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AverageFrameTimeMs;

    /** Peak frame time recorded */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PeakFrameTimeMs;

    /** Number of active physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActivePhysicsBodies;

    /** Number of collision checks per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CollisionChecksPerFrame;

    /** Memory usage in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    /** Performance health status */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceStatus PerformanceStatus;

    FCore_PhysicsPerformanceMetrics()
    {
        AverageFrameTimeMs = 0.0f;
        PeakFrameTimeMs = 0.0f;
        ActivePhysicsBodies = 0;
        CollisionChecksPerFrame = 0;
        MemoryUsageMB = 0.0f;
        PerformanceStatus = EPerformanceStatus::Optimal;
    }
};

/**
 * Core Physics Architectural Integrator
 * 
 * Bridges all physics systems with the Engine Architect's architectural core.
 * Handles registration, validation, performance monitoring, and compliance
 * checking for all physics-related systems in the game.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsArchitecturalIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsArchitecturalIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Register a physics system with the architectural core
     * @param SystemName Name of the physics system
     * @param Priority Initialization priority (lower = earlier)
     * @param bIsCritical Whether system is critical for gameplay
     * @param Dependencies List of system dependencies
     * @param PerformanceBudget Performance budget in ms per frame
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool RegisterPhysicsSystem(const FString& SystemName, int32 Priority, bool bIsCritical, 
                              const TArray<FString>& Dependencies, float PerformanceBudget);

    /**
     * Validate physics system architecture compliance
     * @param SystemName Name of system to validate
     * @return True if system passes all architectural requirements
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool ValidatePhysicsSystemArchitecture(const FString& SystemName);

    /**
     * Update performance metrics for a physics system
     * @param SystemName Name of the system
     * @param Metrics Current performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void UpdatePhysicsPerformanceMetrics(const FString& SystemName, const FCore_PhysicsPerformanceMetrics& Metrics);

    /**
     * Get current performance status for all physics systems
     * @return Array of performance metrics for all registered systems
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    TArray<FCore_PhysicsPerformanceMetrics> GetAllPhysicsPerformanceMetrics() const;

    /**
     * Initialize all registered physics systems in priority order
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void InitializePhysicsSystems();

    /**
     * Shutdown all physics systems in reverse priority order
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void ShutdownPhysicsSystems();

    /**
     * Validate that all physics systems meet architectural standards
     * @return True if all systems pass validation
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool ValidateAllPhysicsSystems();

    /**
     * Get system registration data for a specific physics system
     * @param SystemName Name of the system
     * @return Registration data, or default struct if not found
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    FCore_PhysicsSystemRegistration GetPhysicsSystemRegistration(const FString& SystemName) const;

    /**
     * Check if a physics system is properly registered
     * @param SystemName Name of the system to check
     * @return True if system is registered
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool IsPhysicsSystemRegistered(const FString& SystemName) const;

    /**
     * Get list of all registered physics systems
     * @return Array of system names
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    TArray<FString> GetRegisteredPhysicsSystems() const;

protected:
    /** Reference to the architectural core system */
    UPROPERTY()
    UEng_ArchitecturalCore* ArchitecturalCore;

    /** Map of registered physics systems */
    UPROPERTY()
    TMap<FString, FCore_PhysicsSystemRegistration> RegisteredSystems;

    /** Performance metrics for each system */
    UPROPERTY()
    TMap<FString, FCore_PhysicsPerformanceMetrics> PerformanceMetrics;

    /** Reference to core physics manager */
    UPROPERTY()
    UCore_PhysicsManager* PhysicsManager;

    /** Reference to collision system */
    UPROPERTY()
    UCore_CollisionSystem* CollisionSystem;

    /** Reference to ragdoll system */
    UPROPERTY()
    UCore_RagdollSystem* RagdollSystem;

    /** Reference to destruction system */
    UPROPERTY()
    UCore_DestructionSystem* DestructionSystem;

private:
    /**
     * Register all core physics systems
     */
    void RegisterCorePhysicsSystems();

    /**
     * Validate system dependencies are met
     * @param SystemName Name of system to validate
     * @return True if all dependencies are satisfied
     */
    bool ValidateSystemDependencies(const FString& SystemName) const;

    /**
     * Get initialization order based on priorities and dependencies
     * @return Ordered list of system names for initialization
     */
    TArray<FString> GetInitializationOrder() const;

    /**
     * Check performance compliance for a system
     * @param SystemName Name of system to check
     * @return True if system meets performance requirements
     */
    bool CheckPerformanceCompliance(const FString& SystemName) const;
};