#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "Perf_TerrainPhysicsPerformanceAnalyzer.generated.h"

DECLARE_STATS_GROUP(TEXT("Terrain Physics Performance"), STATGROUP_TerrainPhysics, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Terrain Physics Update"), STAT_TerrainPhysicsUpdate, STATGROUP_TerrainPhysics);
DECLARE_CYCLE_STAT(TEXT("Physics Material Application"), STAT_PhysicsMaterialApplication, STATGROUP_TerrainPhysics);
DECLARE_CYCLE_STAT(TEXT("Collision Detection"), STAT_TerrainCollisionDetection, STATGROUP_TerrainPhysics);
DECLARE_CYCLE_STAT(TEXT("LOD Calculation"), STAT_TerrainPhysicsLOD, STATGROUP_TerrainPhysics);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float PhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CollisionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 TerrainCollisionChecks;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float TargetFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    bool bIsPerformanceOptimal;

    FPerf_TerrainPhysicsMetrics()
        : FrameTime(0.0f)
        , PhysicsTime(0.0f)
        , CollisionTime(0.0f)
        , ActivePhysicsActors(0)
        , TerrainCollisionChecks(0)
        , MemoryUsageMB(0.0f)
        , CPUUsagePercent(0.0f)
        , TargetFrameRate(60.0f)
        , bIsPerformanceOptimal(true)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomePerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    int32 PhysicsActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    float TerrainComplexity;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Performance")
    bool bRequiresOptimization;

    FPerf_BiomePerformanceData()
        : BiomeName(TEXT("Unknown"))
        , BiomeCenter(FVector::ZeroVector)
        , AverageFrameTime(0.0f)
        , PhysicsActorCount(0)
        , TerrainComplexity(1.0f)
        , bRequiresOptimization(false)
    {
    }
};

UENUM(BlueprintType)
enum class EPerf_TerrainOptimizationLevel : uint8
{
    Low         UMETA(DisplayName = "Low Optimization"),
    Medium      UMETA(DisplayName = "Medium Optimization"),
    High        UMETA(DisplayName = "High Optimization"),
    Extreme     UMETA(DisplayName = "Extreme Optimization")
};

/**
 * Terrain Physics Performance Analyzer
 * Monitors and optimizes terrain physics performance across different biomes
 * Ensures 60fps on PC and 30fps on console with terrain physics enabled
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_TerrainPhysicsPerformanceAnalyzer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_TerrainPhysicsPerformanceAnalyzer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance Analysis Methods
    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    void StartPerformanceAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    void StopPerformanceAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    FPerf_TerrainPhysicsMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    void AnalyzeBiomePerformance(const FVector& BiomeCenter, float Radius, const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    TArray<FPerf_BiomePerformanceData> GetBiomePerformanceData() const;

    // Optimization Methods
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeTerrainPhysicsPerformance(EPerf_TerrainOptimizationLevel OptimizationLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void SetTargetFrameRate(float NewTargetFrameRate);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void EnableAdaptiveOptimization(bool bEnable);

    // Monitoring Methods
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    bool IsPerformanceWithinTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetCurrentFrameRate() const;

protected:
    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    FPerf_TerrainPhysicsMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<FPerf_BiomePerformanceData> BiomePerformanceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float PerformanceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableAdaptiveOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_TerrainOptimizationLevel CurrentOptimizationLevel;

    // Analysis State
    UPROPERTY(BlueprintReadOnly, Category = "Analysis State")
    bool bIsAnalysisActive;

    UPROPERTY(BlueprintReadOnly, Category = "Analysis State")
    float AnalysisStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Analysis State")
    int32 FramesSampled;

private:
    // Internal Methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void AnalyzePhysicsActors();
    void CalculateTerrainComplexity(const FVector& Location, float& OutComplexity);
    void ApplyOptimizations(EPerf_TerrainOptimizationLevel Level);
    void CheckAdaptiveOptimization();
    
    // Performance Tracking
    float AccumulatedFrameTime;
    float AccumulatedPhysicsTime;
    float LastUpdateTime;
    TArray<float> FrameTimeHistory;
    
    // Cached References
    UPROPERTY()
    UWorld* CachedWorld;
    
    UPROPERTY()
    ALandscape* CachedLandscape;
    
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
};