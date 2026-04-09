// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Engine/LevelStreaming.h"
#include "WorldPartitionArchitectureManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWorldPartitionArchitecture, Log, All);

/**
 * World Partition Architecture Manager
 * 
 * WORLD PARTITION STANDARDS (as defined by Engine Architect):
 * - All worlds >4km² MUST use World Partition
 * - Cell size: 256m x 256m (configurable based on content density)
 * - Loading range: 512m from player (2 cells in each direction)
 * - Streaming budget: 2GB memory, 16.67ms frame time
 * - HLOD generation: Automatic for distances >1km
 * - Data layers: Separate gameplay, environment, and audio
 * 
 * PERFORMANCE REQUIREMENTS:
 * - Loading time: <5s between regions
 * - Memory overhead: <10% of total budget
 * - Streaming hitches: <1ms per frame
 * - Cell activation: <100ms
 */

UENUM(BlueprintType)
enum class EWorldPartitionMode : uint8
{
    Disabled = 0        UMETA(DisplayName = "Disabled - Traditional level streaming"),
    Standard = 1        UMETA(DisplayName = "Standard - Basic World Partition"),
    Advanced = 2        UMETA(DisplayName = "Advanced - Full World Partition with HLODs"),
    Experimental = 3    UMETA(DisplayName = "Experimental - Cutting-edge features")
};

UENUM(BlueprintType)
enum class EStreamingStrategy : uint8
{
    Distance = 0        UMETA(DisplayName = "Distance-based streaming"),
    Performance = 1     UMETA(DisplayName = "Performance-based streaming"),
    Predictive = 2      UMETA(DisplayName = "Predictive streaming"),
    Hybrid = 3          UMETA(DisplayName = "Hybrid strategy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorldPartitionConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    EWorldPartitionMode Mode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 CellSizeMeters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    float LoadingRangeMeters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    float UnloadingRangeMeters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    EStreamingStrategy StreamingStrategy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    bool bEnableHLODs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    bool bEnableDataLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    float MemoryBudgetMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 MaxConcurrentLoads;

    FWorldPartitionConfiguration()
    {
        Mode = EWorldPartitionMode::Standard;
        CellSizeMeters = 25600; // 256m
        LoadingRangeMeters = 51200.0f; // 512m
        UnloadingRangeMeters = 76800.0f; // 768m
        StreamingStrategy = EStreamingStrategy::Distance;
        bEnableHLODs = true;
        bEnableDataLayers = true;
        MemoryBudgetMB = 2048.0f;
        MaxConcurrentLoads = 4;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStreamingMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 LoadedCells;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 LoadingCells;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 UnloadingCells;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float LoadingBandwidthMBps;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageLoadTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 StreamingHitches;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FDateTime Timestamp;

    FStreamingMetrics()
    {
        LoadedCells = 0;
        LoadingCells = 0;
        UnloadingCells = 0;
        MemoryUsageMB = 0.0f;
        LoadingBandwidthMBps = 0.0f;
        AverageLoadTimeMS = 0.0f;
        StreamingHitches = 0;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorldRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FString RegionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FBox RegionBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    float ComplexityScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    bool bAlwaysLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    TArray<FString> DataLayers;

    FWorldRegion()
    {
        RegionName = TEXT("");
        RegionBounds = FBox(ForceInit);
        ComplexityScore = 1.0f;
        Priority = 100;
        bAlwaysLoaded = false;
    }
};

/**
 * World Partition Architecture Manager
 * Manages World Partition configuration and streaming for large worlds
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorldPartitionArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UWorldPartitionArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Tick(float DeltaTime) override;

    // World Partition Configuration
    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void InitializeWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void ConfigureWorldPartition(const FWorldPartitionConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "World Partition")
    FWorldPartitionConfiguration GetWorldPartitionConfiguration();

    UFUNCTION(BlueprintCallable, Category = "World Partition")
    bool IsWorldPartitionEnabled();

    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void SetWorldPartitionMode(EWorldPartitionMode Mode);

    // Cell Management
    UFUNCTION(BlueprintCallable, Category = "Cell Management")
    void SetCellSize(int32 SizeInMeters);

    UFUNCTION(BlueprintCallable, Category = "Cell Management")
    void SetLoadingRange(float RangeInMeters);

    UFUNCTION(BlueprintCallable, Category = "Cell Management")
    TArray<FVector> GetLoadedCellPositions();

    UFUNCTION(BlueprintCallable, Category = "Cell Management")
    TArray<FVector> GetLoadingCellPositions();

    UFUNCTION(BlueprintCallable, Category = "Cell Management")
    void ForceLoadCell(const FVector& CellPosition);

    UFUNCTION(BlueprintCallable, Category = "Cell Management")
    void ForceUnloadCell(const FVector& CellPosition);

    // Streaming Management
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void SetStreamingStrategy(EStreamingStrategy Strategy);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void EnablePredictiveStreaming(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void SetStreamingBudget(float MemoryBudgetMB, int32 MaxConcurrentLoads);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    FStreamingMetrics GetStreamingMetrics();

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void OptimizeStreamingForPerformance();

    // Region Management
    UFUNCTION(BlueprintCallable, Category = "Region Management")
    void RegisterWorldRegion(const FWorldRegion& Region);

    UFUNCTION(BlueprintCallable, Category = "Region Management")
    void UnregisterWorldRegion(const FString& RegionName);

    UFUNCTION(BlueprintCallable, Category = "Region Management")
    TArray<FWorldRegion> GetWorldRegions();

    UFUNCTION(BlueprintCallable, Category = "Region Management")
    FWorldRegion GetRegionAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Region Management")
    void SetRegionPriority(const FString& RegionName, int32 Priority);

    // HLOD Management
    UFUNCTION(BlueprintCallable, Category = "HLOD")
    void EnableHLODs(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "HLOD")
    void ConfigureHLODSettings(float HLODDistance, int32 HLODLevels);

    UFUNCTION(BlueprintCallable, Category = "HLOD")
    void BuildHLODsForRegion(const FString& RegionName);

    UFUNCTION(BlueprintCallable, Category = "HLOD")
    bool AreHLODsReady();

    // Data Layer Management
    UFUNCTION(BlueprintCallable, Category = "Data Layers")
    void EnableDataLayers(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Data Layers")
    void CreateDataLayer(const FString& LayerName, bool bIsRuntimeLayer);

    UFUNCTION(BlueprintCallable, Category = "Data Layers")
    void SetDataLayerState(const FString& LayerName, bool bLoaded, bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Data Layers")
    TArray<FString> GetActiveDataLayers();

    UFUNCTION(BlueprintCallable, Category = "Data Layers")
    void AssignActorToDataLayer(AActor* Actor, const FString& LayerName);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartStreamingProfiler();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopStreamingProfiler();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GenerateStreamingReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsStreamingPerformanceOptimal();

    // Validation and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    TArray<FString> ValidateWorldPartitionSetup();

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void AnalyzeWorldComplexity();

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    FString GetWorldPartitionStatus();

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void ExportWorldPartitionData(const FString& FilePath);

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FWorldPartitionConfiguration CurrentConfiguration;

    // World regions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Regions")
    TArray<FWorldRegion> WorldRegions;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FStreamingMetrics> StreamingHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bStreamingProfilerActive;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bWorldPartitionInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bHLODsEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bDataLayersEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastUpdateTime;

private:
    // Internal management
    void UpdateStreamingMetrics();
    void ProcessStreamingRequests();
    void ManageMemoryBudget();
    void OptimizeCellLoading();
    
    // Configuration helpers
    void ApplyWorldPartitionSettings();
    void SetupDefaultRegions();
    void ConfigureStreamingSettings();
    
    // Performance optimization
    void AnalyzeStreamingPerformance();
    void AdjustStreamingParameters();
    void PredictStreamingNeeds();
    
    // Validation
    bool ValidateConfiguration();
    void CheckStreamingHealth();
    void LogStreamingEvents();
    
    // Metrics collection
    FStreamingMetrics CollectStreamingMetrics();
    void UpdateStreamingHistory();
    
    // Memory management
    void MonitorMemoryUsage();
    void EnforceMemoryBudget();
    void CleanupUnusedCells();
    
    // Timing
    float UpdateInterval;
    float LastMetricsUpdate;
};