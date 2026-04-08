/**
 * @file PerformanceOptimizer.h
 * @brief Performance optimization system for consciousness physics
 * @author Performance Optimizer Agent
 * @version 1.0
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/ThreadSafeBool.h"
#include "Containers/Queue.h"
#include "Async/ParallelFor.h"
#include "PerformanceOptimizer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceOptimizer, Log, All);

/**
 * Performance optimization levels for consciousness physics
 */
UENUM(BlueprintType)
enum class EPerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"), 
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Potato      UMETA(DisplayName = "Potato Mode")
};

/**
 * LOD (Level of Detail) system for consciousness entities
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FConsciousnessLOD
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxEntities = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableComplexPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableParticleEffects = true;
};

/**
 * Performance metrics tracking
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float PhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveConsciousnessEntities = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 PhysicsCalculationsPerFrame = 0;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float CPUUsagePercent = 0.0f;
};

/**
 * Main performance optimization system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UObject
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

    // Core optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializeOptimizer();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeConsciousnessPhysics();

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UpdateLODSystem(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    FConsciousnessLOD GetLODForDistance(float Distance) const;

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void CleanupUnusedResources();

    // Threading and Async
    UFUNCTION(BlueprintCallable, Category = "Performance|Threading")
    void EnableMultithreading(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance|Threading")
    void ProcessPhysicsAsync();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerformanceLevel GetCurrentPerformanceLevel() const { return CurrentPerformanceLevel; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsOptimizationEnabled() const { return bOptimizationEnabled; }

protected:
    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceLevel CurrentPerformanceLevel = EPerformanceLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bOptimizationEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableMultithreading = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    // LOD Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<FConsciousnessLOD> LODLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODUpdateInterval = 0.1f;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerformanceMetrics CurrentMetrics;

    // Threading
    UPROPERTY()
    FThreadSafeBool bIsProcessingAsync;

    // Timers
    UPROPERTY()
    float LODUpdateTimer = 0.0f;

    UPROPERTY()
    float MetricsUpdateTimer = 0.0f;

private:
    // Internal optimization methods
    void ApplyPerformanceSettings();
    void UpdateFrameRateOptimization();
    void OptimizePhysicsCalculations();
    void ManageEntityCulling();
    
    // Memory optimization
    void OptimizeTextureMemory();
    void OptimizeMeshMemory();
    void OptimizePhysicsMemory();
    
    // Threading helpers
    void ProcessConsciousnessPhysicsParallel();
    void ProcessEntityUpdatesParallel();
    
    // Metrics calculation
    void CalculateFrameMetrics();
    void CalculateMemoryMetrics();
    void CalculateCPUMetrics();
};