#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkeletalMeshActor.h"
#include "Perf_PerformanceManager.generated.h"

// Performance monitoring data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalDrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMemoryUsageMB;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MinFPS = 60.0f;
        MaxFPS = 60.0f;
        ActivePhysicsActors = 0;
        TotalDrawCalls = 0;
        MemoryUsageMB = 0.0f;
        GPUMemoryUsageMB = 0.0f;
    }
};

// LOD configuration structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableAutomaticLOD;

    FPerf_LODConfiguration()
    {
        LOD0Distance = 1000.0f;
        LOD1Distance = 2000.0f;
        LOD2Distance = 5000.0f;
        CullingDistance = 10000.0f;
        bEnableAutomaticLOD = true;
    }
};

// Performance optimization levels
UENUM(BlueprintType)
enum class EPerf_OptimizationLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Potato      UMETA(DisplayName = "Potato Mode")
};

// Performance target platforms
UENUM(BlueprintType)
enum class EPerf_TargetPlatform : uint8
{
    PC_HighEnd      UMETA(DisplayName = "PC High-End"),
    PC_MidRange     UMETA(DisplayName = "PC Mid-Range"),
    PC_LowEnd       UMETA(DisplayName = "PC Low-End"),
    Console_PS5     UMETA(DisplayName = "PlayStation 5"),
    Console_XboxSX  UMETA(DisplayName = "Xbox Series X"),
    Console_Switch  UMETA(DisplayName = "Nintendo Switch")
};

/**
 * Performance Manager Subsystem
 * Handles real-time performance monitoring, LOD management, and optimization
 * Ensures 60fps on PC high-end and 30fps on consoles
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetLODConfiguration(const FPerf_LODConfiguration& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    FPerf_LODConfiguration GetLODConfiguration() const;

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void ApplyLODToActor(AActor* Actor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateAllActorLODs();

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetOptimizationLevel(EPerf_OptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetTargetPlatform(EPerf_TargetPlatform Platform);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeForCurrentPlatform();

    // Physics optimization
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void OptimizePhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void DisableDistantPhysics(float MaxDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnablePhysicsLOD();

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    float GetMemoryUsageMB() const;

    // Debug and profiling
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnablePerformanceHUD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DumpPerformanceStats();

protected:
    // Performance metrics tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_LODConfiguration LODConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_OptimizationLevel CurrentOptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_TargetPlatform CurrentTargetPlatform;

    // Performance monitoring state
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MonitoringInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 FrameCounter;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TotalFrameTime;

    // Target performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS_PC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS_Console;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinAcceptableFPS;

private:
    // Internal performance tracking
    TArray<float> FPSHistory;
    FTimerHandle MonitoringTimerHandle;
    
    // Helper functions
    void InternalUpdateMetrics();
    void CalculateAverageFPS();
    void CheckPerformanceThresholds();
    void ApplyOptimizationSettings();
    void UpdateLODDistances();
    float CalculateActorDistance(AActor* Actor) const;
    void SetActorLODLevel(AActor* Actor, int32 LODLevel);
};