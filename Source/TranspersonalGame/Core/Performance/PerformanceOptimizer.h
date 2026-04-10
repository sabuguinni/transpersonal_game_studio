#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "PerformanceOptimizer.generated.h"

/**
 * Performance metrics structure for tracking frame performance
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    /** Current frames per second */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    /** Frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS;

    /** Game thread time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTimeMS;

    /** Render thread time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTimeMS;

    /** Physics time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTimeMS;

    /** Memory usage in megabytes */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    /** Number of active physics objects */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsObjects;

    /** Number of rendered triangles */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 RenderedTriangles;

    FPerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        FrameTimeMS = 0.0f;
        GameThreadTimeMS = 0.0f;
        RenderThreadTimeMS = 0.0f;
        PhysicsTimeMS = 0.0f;
        MemoryUsageMB = 0.0f;
        ActivePhysicsObjects = 0;
        RenderedTriangles = 0;
    }
};

/**
 * Performance optimization levels
 */
UENUM(BlueprintType)
enum class EPerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (60+ FPS)"),
    High        UMETA(DisplayName = "High (45-60 FPS)"),
    Medium      UMETA(DisplayName = "Medium (30-45 FPS)"),
    Low         UMETA(DisplayName = "Low (15-30 FPS)"),
    Emergency   UMETA(DisplayName = "Emergency (<15 FPS)")
};

/**
 * @class UPerformanceOptimizer
 * @brief Advanced performance optimization system for Transpersonal Game
 * 
 * This component implements dynamic performance optimization based on Richard Fabian's
 * data-oriented design principles and Christer Ericson's real-time optimization techniques.
 * 
 * Key Features:
 * - Real-time FPS monitoring and adaptive quality scaling
 * - LOD-based physics object management
 * - Memory pressure detection and mitigation
 * - Dynamic shadow and rendering quality adjustment
 * - Performance profiling and bottleneck identification
 * 
 * Philosophy:
 * "Performance is not a feature, it's the foundation that allows all other features to exist.
 * A beautiful game that stutters is a broken game."
 * 
 * @author Performance Optimizer #04
 * @date 2024
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * Initialize the performance optimization system
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializeOptimizer();

    /**
     * Get current performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceMetrics GetPerformanceMetrics() const;

    /**
     * Set target frame rate (60fps PC, 30fps console)
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS);

    /**
     * Force performance level adjustment
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerformanceLevel Level);

    /**
     * Enable or disable automatic performance scaling
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAutoScalingEnabled(bool bEnabled);

    /**
     * Optimize physics objects based on distance and importance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsObjects();

    /**
     * Adjust rendering quality based on performance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustRenderingQuality(EPerformanceLevel Level);

    /**
     * Force garbage collection and memory cleanup
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceMemoryCleanup();

    /**
     * Generate performance report
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GeneratePerformanceReport() const;

    /**
     * Get singleton instance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    static UPerformanceOptimizer* GetPerformanceOptimizer(UWorld* World);

protected:
    /**
     * Update performance metrics
     */
    void UpdatePerformanceMetrics(float DeltaTime);

    /**
     * Analyze performance and adjust settings
     */
    void AnalyzeAndOptimize();

    /**
     * Apply LOD optimizations to physics objects
     */
    void ApplyPhysicsLOD();

    /**
     * Adjust shadow quality based on performance
     */
    void AdjustShadowQuality(EPerformanceLevel Level);

    /**
     * Manage texture streaming for memory optimization
     */
    void OptimizeTextureStreaming();

    /**
     * Calculate performance score (0-100)
     */
    float CalculatePerformanceScore() const;

private:
    /** Target frame rate (60fps PC, 30fps console) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate;

    /** Current performance level */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Status", meta = (AllowPrivateAccess = "true"))
    EPerformanceLevel CurrentPerformanceLevel;

    /** Enable automatic performance scaling */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoScalingEnabled;

    /** Enable physics LOD optimization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsLODEnabled;

    /** Maximum physics objects to simulate simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxActivePhysicsObjects;

    /** Distance threshold for physics LOD (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings", meta = (AllowPrivateAccess = "true"))
    float PhysicsLODDistance;

    /** Current performance metrics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Status", meta = (AllowPrivateAccess = "true"))
    FPerformanceMetrics CurrentMetrics;

    /** Frame time history for averaging */
    TArray<float> FrameTimeHistory;

    /** Maximum frame time history size */
    static constexpr int32 MaxFrameTimeHistory = 60;

    /** Time since last optimization check */
    float TimeSinceLastOptimization;

    /** Optimization check interval */
    static constexpr float OptimizationInterval = 1.0f;

    /** Singleton instance */
    static UPerformanceOptimizer* Instance;

    /** Performance thresholds */
    static constexpr float UltraFPSThreshold = 60.0f;
    static constexpr float HighFPSThreshold = 45.0f;
    static constexpr float MediumFPSThreshold = 30.0f;
    static constexpr float LowFPSThreshold = 15.0f;
};