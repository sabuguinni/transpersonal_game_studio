#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Materials/MaterialInterface.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "../SharedTypes.h"
#include "Perf_TerrainPhysicsPerformanceOptimizer.generated.h"

// Terrain physics performance metrics
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TerrainCollisionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsSimulationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaterialCalculationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TerrainCollisionChecks;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeImpact;

    FPerf_TerrainPhysicsMetrics()
        : TerrainCollisionTime(0.0f)
        , PhysicsSimulationTime(0.0f)
        , MaterialCalculationTime(0.0f)
        , ActivePhysicsActors(0)
        , TerrainCollisionChecks(0)
        , FrameTimeImpact(0.0f)
    {}
};

// Terrain physics LOD levels
UENUM(BlueprintType)
enum class EPerf_TerrainPhysicsLOD : uint8
{
    Ultra      UMETA(DisplayName = "Ultra - Full Physics"),
    High       UMETA(DisplayName = "High - Detailed Physics"),
    Medium     UMETA(DisplayName = "Medium - Standard Physics"),
    Low        UMETA(DisplayName = "Low - Simplified Physics"),
    Minimal    UMETA(DisplayName = "Minimal - Basic Collision")
};

// Terrain physics optimization settings
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TerrainPhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPhysicsDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActivePhysicsActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableComplexityReduction;

    FPerf_TerrainPhysicsSettings()
        : MaxPhysicsDistance(5000.0f)
        , LODTransitionDistance(2000.0f)
        , MaxActivePhysicsActors(500)
        , PhysicsUpdateRate(60.0f)
        , bEnableDistanceCulling(true)
        , bEnableComplexityReduction(true)
    {}
};

/**
 * Advanced terrain physics performance optimization system
 * Manages LOD, culling, and performance budgets for terrain physics
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_TerrainPhysicsPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_TerrainPhysicsPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializeTerrainPhysicsOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_TerrainPhysicsMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTerrainPhysicsLOD(EPerf_TerrainPhysicsLOD NewLOD);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_TerrainPhysicsLOD GetCurrentLOD() const { return CurrentLOD; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODBasedOnPerformance();

    // Distance-based optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsActorsByDistance(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantPhysicsActors();

    // Performance budgeting
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinPerformanceBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnforcePerformanceBudget();

    // Terrain-specific optimizations
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainCollision();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SimplifyTerrainMaterials();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ReducePhysicsComplexity();

    // Debug and analysis
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void AnalyzeTerrainPhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceMetrics() const;

protected:
    // Core optimization methods
    void UpdatePhysicsActorLOD(AActor* Actor, float Distance);
    void ApplyLODSettings(EPerf_TerrainPhysicsLOD LOD);
    void MonitorFrameTime(float DeltaTime);
    void CollectPhysicsMetrics();
    void OptimizeBasedOnMetrics();

    // Internal helpers
    TArray<AActor*> GetNearbyPhysicsActors(const FVector& Location, float Radius);
    float CalculatePhysicsComplexity(AActor* Actor);
    void SetActorPhysicsLOD(AActor* Actor, int32 LODLevel);
    bool ShouldCullActor(AActor* Actor, const FVector& PlayerLocation);

private:
    UPROPERTY(EditAnywhere, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    FPerf_TerrainPhysicsSettings OptimizationSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_TerrainPhysicsMetrics CurrentMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_TerrainPhysicsLOD CurrentLOD;

    // Performance tracking
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    TArray<AActor*> TrackedPhysicsActors;

    UPROPERTY()
    TMap<AActor*, int32> ActorLODLevels;

    // Timing
    float LastOptimizationTime;
    float OptimizationInterval;
    float PerformanceBudgetMS;
    
    // Metrics collection
    int32 MetricsCollectionFrames;
    float TotalPhysicsTime;
    float TotalCollisionTime;
};