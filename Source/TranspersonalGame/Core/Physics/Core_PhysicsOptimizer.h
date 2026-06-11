#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsOptimizer.generated.h"

/**
 * Core Physics Optimization System
 * Manages physics performance, LOD systems, and collision optimization
 * for the Transpersonal Game Studio prehistoric survival game.
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
    // === PHYSICS PERFORMANCE SETTINGS ===
    
    /** Maximum number of physics objects to simulate simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    int32 MaxPhysicsObjects = 500;
    
    /** Distance threshold for physics LOD switching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float PhysicsLODDistance = 2000.0f;
    
    /** Enable physics object pooling for performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bEnablePhysicsPooling = true;
    
    /** Physics simulation substeps for accuracy vs performance balance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    int32 PhysicsSubsteps = 2;
    
    // === COLLISION OPTIMIZATION ===
    
    /** Enable collision complexity optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Optimization")
    bool bOptimizeCollisionComplexity = true;
    
    /** Distance for collision detail reduction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Optimization")
    float CollisionLODDistance = 1500.0f;
    
    /** Enable collision shape simplification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Optimization")
    bool bSimplifyCollisionShapes = true;
    
    // === PERFORMANCE MONITORING ===
    
    /** Current physics frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float PhysicsFrameTime = 0.0f;
    
    /** Number of active physics objects */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    int32 ActivePhysicsObjects = 0;
    
    /** Physics memory usage in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float PhysicsMemoryUsage = 0.0f;
    
    // === OPTIMIZATION FUNCTIONS ===
    
    /** Optimize physics settings based on current performance */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsSettings();
    
    /** Update physics LOD based on distance */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UpdatePhysicsLOD();
    
    /** Enable/disable physics for distant objects */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ManageDistantPhysics();
    
    /** Optimize collision complexity for performance */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeCollisionComplexity();
    
    /** Get current physics performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void UpdatePerformanceMetrics();
    
    /** Force physics cleanup and optimization */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ForcePhysicsCleanup();
    
    // === ADAPTIVE PERFORMANCE ===
    
    /** Automatically adjust physics quality based on performance */
    UFUNCTION(BlueprintCallable, Category = "Adaptive Performance")
    void AdaptPhysicsQuality();
    
    /** Set physics quality level (0=Low, 1=Medium, 2=High, 3=Ultra) */
    UFUNCTION(BlueprintCallable, Category = "Adaptive Performance")
    void SetPhysicsQualityLevel(int32 QualityLevel);

private:
    // === INTERNAL OPTIMIZATION DATA ===
    
    /** Timer for performance monitoring updates */
    float PerformanceUpdateTimer = 0.0f;
    
    /** Target frame time for physics (16.67ms for 60fps) */
    float TargetPhysicsFrameTime = 16.67f;
    
    /** Current physics quality level */
    int32 CurrentQualityLevel = 2;
    
    /** Physics objects currently being tracked */
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
    
    /** Last performance measurement time */
    float LastPerformanceMeasurement = 0.0f;
    
    // === INTERNAL HELPER FUNCTIONS ===
    
    /** Scan for physics objects in the world */
    void ScanPhysicsObjects();
    
    /** Apply LOD settings to a specific actor */
    void ApplyPhysicsLOD(AActor* Actor, float Distance);
    
    /** Calculate distance-based physics quality */
    int32 CalculatePhysicsQuality(float Distance);
    
    /** Update physics world settings */
    void UpdatePhysicsWorldSettings();
};