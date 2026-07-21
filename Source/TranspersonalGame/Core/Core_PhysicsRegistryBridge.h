#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_SystemsRegistry.h"
#include "Core_PhysicsManager.h"
#include "Core_CollisionSystem.h"
#include "Core_RagdollSystem.h"
#include "Core_DestructionSystem.h"
#include "Core_TerrainPhysics.h"
#include "Core_FluidDynamics.h"
#include "Core_MaterialPhysics.h"
#include "Core_VehiclePhysics.h"
#include "Core_PhysicsRegistryBridge.generated.h"

/**
 * Core Physics Registry Bridge
 * 
 * Central integration point between all Core physics systems and the Engine Architect's
 * Systems Registry. Manages registration, dependency resolution, and performance monitoring
 * for all physics-related systems in the game.
 * 
 * This bridge ensures that all physics systems follow the architectural standards
 * defined by the Engine Architect while maintaining optimal performance and proper
 * initialization order.
 * 
 * Author: Core Systems Programmer (Agent #3)
 * Integration: Engine Architect Systems Registry v1.0
 * Performance Target: <2ms per frame for all physics operations
 */

UENUM(BlueprintType)
enum class ECore_PhysicsSystemPriority : uint8
{
    Critical = 0,      // Core physics manager - must initialize first
    High = 1,          // Collision and material physics - core gameplay
    Medium = 2,        // Ragdoll, destruction, terrain - visual/gameplay
    Low = 3,           // Fluid dynamics, vehicle physics - specialized
    Debug = 4          // Performance monitoring and debugging tools
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSystemRegistration
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    ECore_PhysicsSystemPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    float PerformanceBudgetMS;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    bool bIsArchitecturallyCompliant;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    FDateTime RegistrationTime;

    FCore_PhysicsSystemRegistration()
    {
        SystemName = TEXT("");
        Priority = ECore_PhysicsSystemPriority::Medium;
        Dependencies = TArray<FString>();
        PerformanceBudgetMS = 1.0f;
        bIsArchitecturallyCompliant = false;
        RegistrationTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float CurrentFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float AverageFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PeakFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 CollisionChecksPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    bool bPerformanceBudgetExceeded;

    FCore_PhysicsPerformanceMetrics()
    {
        CurrentFrameTimeMS = 0.0f;
        AverageFrameTimeMS = 0.0f;
        PeakFrameTimeMS = 0.0f;
        ActivePhysicsBodies = 0;
        CollisionChecksPerFrame = 0;
        bPerformanceBudgetExceeded = false;
    }
};

/**
 * UCore_PhysicsRegistryBridge
 * 
 * Game Instance Subsystem that serves as the central bridge between all Core physics
 * systems and the Engine Architect's Systems Registry. Handles:
 * 
 * - Automatic registration of all physics systems with proper dependencies
 * - Performance monitoring and budget enforcement
 * - Architectural compliance validation
 * - Cross-system communication for physics events
 * - Integration with Engine Architect's performance profiles
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsRegistryBridge : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsRegistryBridge();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Register all Core physics systems with the Systems Registry
     * Called automatically during subsystem initialization
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Registry")
    void RegisterAllPhysicsSystems();

    /**
     * Validate that all physics systems meet architectural compliance standards
     * Returns true if all systems pass validation
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Registry")
    bool ValidatePhysicsArchitecturalCompliance();

    /**
     * Update performance metrics for all registered physics systems
     * Called every frame to monitor performance budget compliance
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Registry")
    void UpdatePhysicsPerformanceMetrics();

    /**
     * Get current performance metrics for the physics subsystem
     */
    UFUNCTION(BlueprintPure, Category = "Physics Registry")
    FCore_PhysicsPerformanceMetrics GetPhysicsPerformanceMetrics() const;

    /**
     * Check if physics systems are operating within performance budget
     */
    UFUNCTION(BlueprintPure, Category = "Physics Registry")
    bool IsWithinPerformanceBudget() const;

    /**
     * Get list of all registered physics systems
     */
    UFUNCTION(BlueprintPure, Category = "Physics Registry")
    TArray<FCore_PhysicsSystemRegistration> GetRegisteredPhysicsSystems() const;

    /**
     * Send a message to all physics systems through the Systems Registry
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Registry")
    void BroadcastPhysicsSystemMessage(const FString& MessageType, const FString& MessageData);

    /**
     * Emergency shutdown of all physics systems if performance budget is critically exceeded
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Registry", CallInEditor)
    void EmergencyPhysicsShutdown();

protected:
    /**
     * Reference to the Engine Architect's Systems Registry
     */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    class UEng_SystemsRegistry* SystemsRegistry;

    /**
     * All registered physics systems with their metadata
     */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    TArray<FCore_PhysicsSystemRegistration> RegisteredSystems;

    /**
     * Current performance metrics
     */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    FCore_PhysicsPerformanceMetrics CurrentMetrics;

    /**
     * Performance budget in milliseconds (set by Engine Architect profile)
     */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    float PerformanceBudgetMS;

    /**
     * Whether the bridge has been successfully initialized
     */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Registry")
    bool bIsInitialized;

private:
    /**
     * Register a single physics system with proper dependency resolution
     */
    void RegisterPhysicsSystem(const FString& SystemName, ECore_PhysicsSystemPriority Priority, 
                              const TArray<FString>& Dependencies, float BudgetMS);

    /**
     * Validate architectural compliance for a specific system
     */
    bool ValidateSystemCompliance(const FString& SystemName);

    /**
     * Calculate current frame performance metrics
     */
    void CalculatePerformanceMetrics();

    // Performance tracking
    double LastFrameTime;
    TArray<float> FrameTimeHistory;
    static constexpr int32 MaxFrameHistory = 60;
};