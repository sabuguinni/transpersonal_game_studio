#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsOptimizer.generated.h"

/**
 * Core Physics Optimizer - Manages physics performance and LOD systems
 * Optimizes physics simulation based on distance, importance, and performance budget
 * Part of the Core Systems module for prehistoric survival game
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === PHYSICS LOD SYSTEM ===
    
    /** Distance thresholds for physics LOD levels */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float HighDetailDistance = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float MediumDetailDistance = 2500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float LowDetailDistance = 5000.0f;
    
    /** Maximum number of high-detail physics objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    int32 MaxHighDetailObjects = 50;
    
    /** Maximum number of medium-detail physics objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    int32 MaxMediumDetailObjects = 100;
    
    // === PERFORMANCE MONITORING ===
    
    /** Target physics frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetPhysicsFrameTime = 8.33f; // 120Hz physics
    
    /** Current physics frame time */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentPhysicsFrameTime = 0.0f;
    
    /** Number of active physics objects */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsObjects = 0;
    
    /** Physics performance budget (0.0 to 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsPerformanceBudget = 1.0f;
    
    // === OPTIMIZATION SETTINGS ===
    
    /** Enable automatic physics LOD */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePhysicsLOD = true;
    
    /** Enable physics object culling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePhysicsCulling = true;
    
    /** Enable adaptive physics timestep */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptiveTimestep = true;
    
    /** Minimum physics timestep */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MinPhysicsTimestep = 0.008333f; // 120Hz
    
    /** Maximum physics timestep */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxPhysicsTimestep = 0.033333f; // 30Hz
    
    // === PHYSICS OPTIMIZATION FUNCTIONS ===
    
    /** Update physics LOD for all objects */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UpdatePhysicsLOD();
    
    /** Optimize physics settings based on performance */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsSettings();
    
    /** Get current physics performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void GetPhysicsPerformanceMetrics(float& FrameTime, int32& ObjectCount, float& Budget);
    
    /** Set physics quality level (0=Low, 1=Medium, 2=High) */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsQualityLevel(int32 QualityLevel);
    
    /** Enable/disable physics for specific object */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetObjectPhysicsEnabled(AActor* Actor, bool bEnabled);
    
    /** Get recommended physics settings for current performance */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void GetRecommendedPhysicsSettings(float& Gravity, float& Timestep, int32& MaxObjects);

private:
    // === INTERNAL OPTIMIZATION DATA ===
    
    /** Cached reference to physics settings */
    UPROPERTY()
    UPhysicsSettings* PhysicsSettings;
    
    /** List of physics objects being tracked */
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsObjects;
    
    /** Performance history for adaptive optimization */
    TArray<float> PerformanceHistory;
    
    /** Last optimization update time */
    float LastOptimizationTime = 0.0f;
    
    /** Optimization update interval */
    float OptimizationInterval = 1.0f;
    
    // === INTERNAL OPTIMIZATION FUNCTIONS ===
    
    /** Update performance metrics */
    void UpdatePerformanceMetrics();
    
    /** Apply physics LOD to specific object */
    void ApplyPhysicsLOD(AActor* Actor, int32 LODLevel);
    
    /** Calculate physics performance budget */
    float CalculatePerformanceBudget();
    
    /** Adjust physics timestep based on performance */
    void AdjustPhysicsTimestep();
    
    /** Cull distant physics objects */
    void CullDistantPhysicsObjects();
    
    /** Find all physics objects in world */
    void RefreshPhysicsObjectList();
};