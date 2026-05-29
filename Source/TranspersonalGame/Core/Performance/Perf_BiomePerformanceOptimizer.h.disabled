#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Perf_BiomePerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_BiomePerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomePerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPhysicsActors = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePhysicsOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableCullingOptimization = true;

    FPerf_BiomePerformanceSettings()
    {
        MaxPhysicsActors = 50.0f;
        CullingDistance = 5000.0f;
        LODDistance = 2000.0f;
        UpdateFrequency = 0.1f;
        bEnablePhysicsOptimization = true;
        bEnableCullingOptimization = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BiomePerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActivePhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PhysicsSimulationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float RenderingTime = 0.0f;

    FPerf_BiomePerformanceMetrics()
    {
        ActivePhysicsActors = 0;
        CulledActors = 0;
        CurrentFrameTime = 0.0f;
        AverageFrameTime = 0.0f;
        PhysicsSimulationTime = 0.0f;
        RenderingTime = 0.0f;
    }
};

/**
 * Biome-specific performance optimization component
 * Optimizes performance for different biomes (Savanna, Swamp, Forest, Desert, Mountain)
 * Manages LOD, culling, and physics optimization based on biome characteristics
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_BiomePerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_BiomePerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance level management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_BiomePerformanceLevel NewLevel);

    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_BiomePerformanceLevel GetPerformanceLevel() const { return CurrentPerformanceLevel; }

    // Biome-specific optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyBiomeSettings(const FPerf_BiomePerformanceSettings& Settings);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_BiomePerformanceMetrics GetPerformanceMetrics() const { return CurrentMetrics; }

    // Optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLOD();

    // Automatic optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsAutoOptimizationEnabled() const { return bAutoOptimizationEnabled; }

protected:
    // Performance settings for each biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    TMap<EBiomeType, FPerf_BiomePerformanceSettings> BiomeSettings;

    // Current performance level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_BiomePerformanceLevel CurrentPerformanceLevel;

    // Current biome type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EBiomeType CurrentBiome;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_BiomePerformanceMetrics CurrentMetrics;

    // Auto optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    bool bAutoOptimizationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    float OptimizationThreshold;

    // Internal tracking
    UPROPERTY()
    TArray<AActor*> TrackedActors;

    UPROPERTY()
    float LastOptimizationTime;

    UPROPERTY()
    TArray<float> FrameTimeHistory;

private:
    // Helper functions
    void InitializeBiomeSettings();
    void UpdateFrameTimeHistory(float DeltaTime);
    float CalculateAverageFrameTime() const;
    void PerformAutoOptimization();
    void OptimizeActorsInRadius(float Radius);
    void UpdateActorLOD(AActor* Actor, float Distance);
    bool ShouldCullActor(AActor* Actor, float Distance) const;
    void GatherPerformanceData();
};