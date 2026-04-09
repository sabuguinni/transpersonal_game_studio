#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "WorldPartition/WorldPartition.h"
#include "ProceduralWorldSystemV6.generated.h"

/**
 * Advanced Procedural World System V6
 * Integrates all world generation systems with performance optimization
 * Uses UE5.5 PCG Framework with GPU acceleration and hierarchical generation
 */

UENUM(BlueprintType)
enum class EWorldGenerationPhase : uint8
{
    Initialization      UMETA(DisplayName = "Initialization"),
    TerrainGeneration   UMETA(DisplayName = "Terrain Generation"),
    BiomeDistribution   UMETA(DisplayName = "Biome Distribution"),
    WaterSystemGeneration UMETA(DisplayName = "Water System Generation"),
    VegetationPlacement UMETA(DisplayName = "Vegetation Placement"),
    DetailGeneration    UMETA(DisplayName = "Detail Generation"),
    Optimization        UMETA(DisplayName = "Optimization"),
    Complete           UMETA(DisplayName = "Complete")
};

USTRUCT(BlueprintType)
struct FWorldGenerationSettings
{
    GENERATED_BODY()

    /** World size in square kilometers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    float WorldSizeKm = 200.0f;

    /** Landscape resolution (must be valid UE5 size) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 LandscapeResolution = 8129; // 8129x8129 for 200km² world

    /** Maximum height in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxHeightM = 2500.0f;

    /** Enable GPU acceleration for PCG */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseGPUAcceleration = true;

    /** Enable hierarchical generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseHierarchicalGeneration = true;

    /** World Partition cell size in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float CellSizeM = 512.0f;

    /** Loading range in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float LoadingRangeM = 1536.0f;

    /** Enable runtime generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableRuntimeGeneration = true;

    /** Seed for procedural generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 WorldSeed = 12345;
};

USTRUCT(BlueprintType)
struct FPerformanceMetrics
{
    GENERATED_BODY()

    /** Current generation phase */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EWorldGenerationPhase CurrentPhase = EWorldGenerationPhase::Initialization;

    /** Generation progress (0.0 to 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GenerationProgress = 0.0f;

    /** Memory usage in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB = 0.0f;

    /** Generation time in seconds */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GenerationTimeSeconds = 0.0f;

    /** Number of active PCG components */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActivePCGComponents = 0;

    /** Current frame rate */
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFPS = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProceduralWorldSystemV6 : public AActor
{
    GENERATED_BODY()

public:
    AProceduralWorldSystemV6();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Root scene component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    /** Master PCG component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPCGComponent* MasterPCGComponent;

    /** World generation settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    FWorldGenerationSettings WorldSettings;

    /** Performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerformanceMetrics PerformanceMetrics;

    /** Master world generation graph */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    class UPCGGraph* MasterWorldGraph;

    /** Terrain generation graph */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    class UPCGGraph* TerrainGraph;

    /** Biome distribution graph */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    class UPCGGraph* BiomeGraph;

    /** Water system graph */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    class UPCGGraph* WaterSystemGraph;

    /** Vegetation graph */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    class UPCGGraph* VegetationGraph;

    /** Generated landscape reference */
    UPROPERTY(BlueprintReadOnly, Category = "Generated World")
    class ALandscape* GeneratedLandscape;

public:
    /** Start world generation process */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void StartWorldGeneration();

    /** Stop world generation process */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void StopWorldGeneration();

    /** Clear generated world */
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearGeneratedWorld();

    /** Get current generation progress */
    UFUNCTION(BlueprintPure, Category = "World Generation")
    float GetGenerationProgress() const { return PerformanceMetrics.GenerationProgress; }

    /** Get current generation phase */
    UFUNCTION(BlueprintPure, Category = "World Generation")
    EWorldGenerationPhase GetCurrentPhase() const { return PerformanceMetrics.CurrentPhase; }

    /** Check if generation is complete */
    UFUNCTION(BlueprintPure, Category = "World Generation")
    bool IsGenerationComplete() const { return PerformanceMetrics.CurrentPhase == EWorldGenerationPhase::Complete; }

    /** Get performance metrics */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceMetrics GetPerformanceMetrics() const { return PerformanceMetrics; }

protected:
    /** Initialize world generation system */
    void InitializeWorldGeneration();

    /** Setup PCG components */
    void SetupPCGComponents();

    /** Configure World Partition */
    void ConfigureWorldPartition();

    /** Generate base terrain */
    void GenerateTerrain();

    /** Distribute biomes */
    void DistributeBiomes();

    /** Generate water systems */
    void GenerateWaterSystems();

    /** Place vegetation */
    void PlaceVegetation();

    /** Generate detail objects */
    void GenerateDetails();

    /** Optimize generated world */
    void OptimizeWorld();

    /** Update performance metrics */
    void UpdatePerformanceMetrics();

    /** Calculate memory usage */
    float CalculateMemoryUsage();

    /** Calculate current FPS */
    float CalculateCurrentFPS();

private:
    /** Generation start time */
    double GenerationStartTime;

    /** Is generation in progress */
    bool bGenerationInProgress;

    /** Generation timer handle */
    FTimerHandle GenerationTimerHandle;
};