#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_PhysicsPerformanceIntegrator.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsLODLevel : uint8
{
    HighDetail      UMETA(DisplayName = "High Detail"),
    MediumDetail    UMETA(DisplayName = "Medium Detail"),
    LowDetail       UMETA(DisplayName = "Low Detail"),
    NoPhysics       UMETA(DisplayName = "No Physics")
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 RagdollBodies = 0;

    FPerf_PhysicsPerformanceMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActivePhysicsObjects = 0;
        CollisionChecks = 0;
        MemoryUsageMB = 0.0f;
        AverageFrameRate = 60.0f;
        RagdollBodies = 0;
    }
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float HighDetailDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MediumDetailDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float LowDetailDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxPhysicsObjects = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsFrameBudget = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePhysicsThreading = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 PhysicsThreadCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAsyncPhysics = true;

    FPerf_PhysicsOptimizationSettings()
    {
        HighDetailDistance = 3000.0f;
        MediumDetailDistance = 8000.0f;
        LowDetailDistance = 15000.0f;
        MaxPhysicsObjects = 100;
        PhysicsFrameBudget = 16.67f;
        bEnablePhysicsThreading = true;
        PhysicsThreadCount = 4;
        bEnableAsyncPhysics = true;
    }
};

/**
 * Performance integrator specifically designed for physics systems optimization
 * Manages physics LOD, collision optimization, threading, and performance monitoring
 * Integrates with Core Systems' DinosaurPhysicsIntegrator for optimal performance
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Performance Management
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void InitializePhysicsPerformanceSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsThreading();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ManagePhysicsBudget();

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    EPerf_PhysicsLODLevel CalculatePhysicsLOD(AActor* Actor, const FVector& ViewerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void ApplyPhysicsLOD(AActor* Actor, EPerf_PhysicsLODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void UpdateAllActorsLOD();

    // Collision Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeCollisionSettings(UPrimitiveComponent* Component, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void CreateOptimizationZones();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UpdateCollisionOptimization();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FPerf_PhysicsPerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    bool IsPerformanceWithinBudget() const;

    // Ragdoll Optimization
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void OptimizeRagdollPerformance(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void EnableRagdollLOD(USkeletalMeshComponent* SkeletalMesh, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void DisableDistantRagdolls();

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void OptimizePhysicsMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void CleanupUnusedPhysicsObjects();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    float GetPhysicsMemoryUsage() const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugDrawPhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugDrawOptimizationZones();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPerformanceMetrics();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoringActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MonitoringStartTime = 0.0f;

    // LOD Management
    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    TArray<AActor*> TrackedActors;

    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    TMap<AActor*, EPerf_PhysicsLODLevel> ActorLODLevels;

    // Optimization Zones
    UPROPERTY(BlueprintReadOnly, Category = "Optimization")
    TArray<AActor*> OptimizationZones;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxPhysicsFrameTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxMemoryUsageMB = 512.0f;

private:
    // Internal tracking
    float LastUpdateTime = 0.0f;
    int32 FrameCounter = 0;
    float AccumulatedFrameTime = 0.0f;
    
    // Helper functions
    void UpdatePerformanceMetrics();
    void ApplyOptimizationBasedOnPerformance();
    AActor* GetPlayerViewpoint() const;
    float CalculateDistanceToViewer(AActor* Actor) const;
    void ConfigurePhysicsSettings();
};