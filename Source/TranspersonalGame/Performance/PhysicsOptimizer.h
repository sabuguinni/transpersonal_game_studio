/**
 * @file PhysicsOptimizer.h
 * @brief Performance optimization system for consciousness physics and large world simulation
 * 
 * Handles LOD systems, spatial partitioning, and adaptive quality for 50km² world
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "PhysicsOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPhysicsLOD : uint8
{
    Highest     UMETA(DisplayName = "Highest Quality"),
    High        UMETA(DisplayName = "High Quality"), 
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

USTRUCT(BlueprintType)
struct FPhysicsLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MaxDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxSimulatedObjects = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float UpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableConsciousnessPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float ConsciousnessFieldResolution = 1.0f;

    FPhysicsLODSettings()
    {
        MaxDistance = 1000.0f;
        MaxSimulatedObjects = 100;
        UpdateFrequency = 60.0f;
        bEnableConsciousnessPhysics = true;
        ConsciousnessFieldResolution = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActivePhysicsObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ConsciousnessFieldUpdates = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CPUUsagePercent = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePhysicsLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPhysicsLOD CalculateLODLevel(float Distance, int32 ObjectComplexity);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalLODSettings(const FPhysicsLODSettings& Settings);

    // Spatial Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializeSpatialPartitioning(float WorldSize, int32 GridResolution);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<AActor*> GetNearbyPhysicsObjects(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateSpatialGrid(AActor* Actor, const FVector& OldLocation, const FVector& NewLocation);

    // Consciousness Physics Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeConsciousnessField(const FVector& FocusPoint, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdaptiveConsciousnessResolution(float TargetFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantConsciousnessEffects(const FVector& PlayerLocation);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupUnusedPhysicsData();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void PreloadCriticalPhysicsAssets(const TArray<FString>& AssetPaths);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnloadDistantPhysicsAssets(const FVector& PlayerLocation, float UnloadDistance);

protected:
    // LOD Settings per level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TMap<EPhysicsLOD, FPhysicsLODSettings> LODSettings;

    // Spatial partitioning grid
    UPROPERTY()
    TArray<TArray<AActor*>> SpatialGrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    float GridCellSize = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
    int32 GridResolution = 50;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPhysicsFrameTime = 8.33f; // ~120fps physics

    // Current metrics
    UPROPERTY()
    FPerformanceMetrics CurrentMetrics;

    // Optimization state
    UPROPERTY()
    bool bAdaptiveOptimization = true;

    UPROPERTY()
    float LastOptimizationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationInterval = 1.0f;

private:
    void UpdatePerformanceMetrics();
    void AdaptLODBasedOnPerformance();
    int32 GetGridIndex(const FVector& Location) const;
    void InitializeLODSettings();
    void OptimizePhysicsSubstep(float DeltaTime);
};