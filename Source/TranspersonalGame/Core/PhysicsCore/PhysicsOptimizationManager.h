#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "SharedTypes.h"
#include "PhysicsOptimizationManager.generated.h"

class UPhysicsLODManager;
class UPhysicsPerformanceProfiler;
class UTerrainDeformationSystem;

/**
 * Core physics optimization data structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsOptimizationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxPhysicsObjects = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float CullingDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAsyncPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PerformanceThreshold = 16.67f; // 60 FPS target

    FCore_PhysicsOptimizationData()
    {
        TargetFrameRate = 60.0f;
        MaxPhysicsObjects = 500;
        PhysicsTickRate = 60.0f;
        CullingDistance = 2000.0f;
        bEnableLODOptimization = true;
        bEnableAsyncPhysics = true;
        PerformanceThreshold = 16.67f;
    }
};

/**
 * Physics performance metrics tracking
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SimulatingBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsage = 0.0f;

    FCore_PhysicsPerformanceMetrics()
    {
        CurrentFrameTime = 0.0f;
        PhysicsTime = 0.0f;
        ActivePhysicsObjects = 0;
        SimulatingBodies = 0;
        MemoryUsage = 0.0f;
        CPUUsage = 0.0f;
    }
};

/**
 * Physics optimization level enumeration
 */
UENUM(BlueprintType)
enum class ECore_PhysicsOptimizationLevel : uint8
{
    None        UMETA(DisplayName = "No Optimization"),
    Low         UMETA(DisplayName = "Low Optimization"),
    Medium      UMETA(DisplayName = "Medium Optimization"),
    High        UMETA(DisplayName = "High Optimization"),
    Extreme     UMETA(DisplayName = "Extreme Optimization")
};

/**
 * Advanced Physics Optimization Manager
 * Manages physics performance, LOD systems, and optimization strategies
 * for the prehistoric survival game environment
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsOptimizationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPhysicsOptimizationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core optimization functions
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void InitializeOptimization();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UpdateOptimization(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetOptimizationLevel(ECore_PhysicsOptimizationLevel Level);

    UFUNCTION(BlueprintPure, Category = "Physics Optimization")
    ECore_PhysicsOptimizationLevel GetOptimizationLevel() const { return CurrentOptimizationLevel; }

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintPure, Category = "Performance")
    FCore_PhysicsPerformanceMetrics GetPerformanceMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceOptimal() const;

    // Physics object management
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void OptimizePhysicsObjects();

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void CullDistantPhysicsObjects();

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    int32 GetActivePhysicsObjectCount() const;

    // LOD system integration
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void EnablePhysicsLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdatePhysicsLOD();

    UFUNCTION(BlueprintPure, Category = "LOD")
    bool IsPhysicsLODEnabled() const { return bPhysicsLODEnabled; }

    // Async physics management
    UFUNCTION(BlueprintCallable, Category = "Async Physics")
    void EnableAsyncPhysics(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Async Physics")
    bool IsAsyncPhysicsEnabled() const { return bAsyncPhysicsEnabled; }

    // Memory optimization
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void OptimizePhysicsMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void CleanupUnusedPhysicsData();

    UFUNCTION(BlueprintPure, Category = "Memory")
    float GetPhysicsMemoryUsage() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetOptimizationData(const FCore_PhysicsOptimizationData& NewData);

    UFUNCTION(BlueprintPure, Category = "Configuration")
    FCore_PhysicsOptimizationData GetOptimizationData() const { return OptimizationData; }

    // Debug and testing
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void RunOptimizationTest();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleDebugVisualization();

protected:
    // Core optimization data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsOptimizationData OptimizationData;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (AllowPrivateAccess = "true"))
    ECore_PhysicsOptimizationLevel CurrentOptimizationLevel;

    // System references
    UPROPERTY()
    UPhysicsLODManager* LODManager;

    UPROPERTY()
    UPhysicsPerformanceProfiler* PerformanceProfiler;

    // State tracking
    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    bool bIsOptimizationActive;

    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoringPerformance;

    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsLODEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    bool bAsyncPhysicsEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    bool bDebugVisualizationEnabled;

    // Performance tracking
    UPROPERTY()
    float LastOptimizationTime;

    UPROPERTY()
    float OptimizationInterval;

    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    int32 MaxFrameHistorySize;

private:
    // Internal optimization methods
    void ApplyOptimizationLevel();
    void UpdatePerformanceMetrics(float DeltaTime);
    void CheckPerformanceThresholds();
    void OptimizePhysicsSettings();
    void ManagePhysicsObjectLimits();
    void UpdateAsyncPhysicsSettings();

    // Helper functions
    void CollectPhysicsObjects();
    void CalculateOptimalSettings();
    bool ShouldOptimizeThisFrame() const;
    void LogOptimizationChanges() const;

    // Cached data
    TArray<TWeakObjectPtr<AActor>> CachedPhysicsActors;
    float LastPerformanceCheck;
    bool bNeedsOptimizationUpdate;
};