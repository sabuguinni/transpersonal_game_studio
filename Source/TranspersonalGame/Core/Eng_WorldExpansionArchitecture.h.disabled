#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Engine/WorldPartition/WorldPartition.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_WorldExpansionArchitecture.generated.h"

/**
 * ENGINE ARCHITECT CYCLE 007 - WORLD EXPANSION ARCHITECTURE
 * 
 * Critical system for 10km² landscape expansion required for $1800 asset purchase.
 * Defines the technical architecture for large-scale world management using:
 * 1. World Partition for streaming 10km² terrain
 * 2. Biome distribution system for 5 distinct regions
 * 3. Asset loading pipeline for commercial dinosaur/environment assets
 * 4. Performance optimization for massive world scale
 * 
 * MISSION CRITICAL: This enables Agent #5 to expand landscape to 10km²
 */

// World Scale Definitions
UENUM(BlueprintType)
enum class EEng_WorldScale : uint8
{
    Small_1km       UMETA(DisplayName = "Small (1km²)"),
    Medium_4km      UMETA(DisplayName = "Medium (4km²)"),
    Large_10km      UMETA(DisplayName = "Large (10km²)"),
    Massive_25km    UMETA(DisplayName = "Massive (25km²)"),
    Unlimited       UMETA(DisplayName = "Unlimited")
};

// Biome Distribution Strategy
UENUM(BlueprintType)
enum class EEng_BiomeDistribution : uint8
{
    Random          UMETA(DisplayName = "Random"),
    Geographic      UMETA(DisplayName = "Geographic"),
    Procedural      UMETA(DisplayName = "Procedural"),
    Designed        UMETA(DisplayName = "Designed"),
    Hybrid          UMETA(DisplayName = "Hybrid")
};

// Asset Loading Strategy
UENUM(BlueprintType)
enum class EEng_AssetLoadStrategy : uint8
{
    Immediate       UMETA(DisplayName = "Immediate"),
    Streaming       UMETA(DisplayName = "Streaming"),
    OnDemand        UMETA(DisplayName = "On Demand"),
    Predictive      UMETA(DisplayName = "Predictive"),
    Hybrid          UMETA(DisplayName = "Hybrid")
};

// World Partition Configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WorldPartitionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    bool bEnableWorldPartition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 GridCellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 LoadingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 UnloadingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    bool bEnableHLODs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 HLODLevels;

    FEng_WorldPartitionConfig()
    {
        bEnableWorldPartition = true;
        GridCellSize = 25600; // 256m cells for 10km² world
        LoadingRange = 3;     // Load 3 cells around player
        UnloadingRange = 5;   // Unload beyond 5 cells
        bEnableHLODs = true;
        HLODLevels = 3;
    }
};

// Biome Region Definition
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D RegionCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D RegionSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> RequiredAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsCommercialAssetReady;

    FEng_BiomeRegion()
    {
        BiomeType = EBiomeType::Temperate_Forest;
        RegionCenter = FVector2D::ZeroVector;
        RegionSize = FVector2D(200000, 200000); // 2km x 2km region
        BlendRadius = 50000.0f; // 500m blend
        bIsCommercialAssetReady = false;
    }
};

// Asset Integration Pipeline
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_AssetPipeline
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    FString AssetPackageName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    FString SourcePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    FString TargetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    EEng_AssetLoadStrategy LoadStrategy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    bool bIsCommercialAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    float EstimatedCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    bool bIntegrationTested;

    FEng_AssetPipeline()
    {
        AssetPackageName = "Unknown";
        LoadStrategy = EEng_AssetLoadStrategy::Streaming;
        bIsCommercialAsset = false;
        EstimatedCost = 0.0f;
        bIntegrationTested = false;
    }
};

/**
 * WORLD EXPANSION ARCHITECTURE SUBSYSTEM
 * 
 * Manages the technical architecture for expanding the game world to 10km²
 * and integrating commercial assets worth $1800 (TurboSquid dinosaurs + RealBiomes)
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_WorldExpansionArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World Scale Management
    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool ConfigureWorldPartition(const FEng_WorldPartitionConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool ExpandWorldTo10km();

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    EEng_WorldScale GetCurrentWorldScale();

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool IsWorldReadyForCommercialAssets();

    // Biome Distribution
    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool SetupBiomeDistribution(const TArray<FEng_BiomeRegion>& BiomeRegions);

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    FEng_BiomeRegion GetBiomeRegion(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    TArray<FEng_BiomeRegion> GetAllBiomeRegions();

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool ValidateBiomeDistribution();

    // Asset Integration Pipeline
    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool RegisterAssetPipeline(const FEng_AssetPipeline& Pipeline);

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool TestFBXImportPipeline(const FString& TestAssetPath);

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    TArray<FEng_AssetPipeline> GetCommercialAssetPipelines();

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool AreCommercialAssetCriteriaMet();

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool OptimizeForLargeScale();

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    float GetWorldStreamingPerformance();

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool ValidatePerformanceTargets();

    // Validation & Diagnostics
    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    bool ValidateWorldExpansionReadiness();

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    TArray<FString> GetExpansionBlockers();

    UFUNCTION(BlueprintCallable, Category = "World Expansion")
    FString GenerateExpansionReport();

protected:
    // World Partition Configuration
    UPROPERTY(EditAnywhere, Category = "World Expansion")
    FEng_WorldPartitionConfig WorldPartitionConfig;

    // Current world scale
    UPROPERTY(BlueprintReadOnly, Category = "World Expansion")
    EEng_WorldScale CurrentWorldScale;

    // Biome distribution
    UPROPERTY(EditAnywhere, Category = "World Expansion")
    TArray<FEng_BiomeRegion> BiomeRegions;

    UPROPERTY(EditAnywhere, Category = "World Expansion")
    EEng_BiomeDistribution BiomeDistributionStrategy;

    // Asset pipelines
    UPROPERTY(EditAnywhere, Category = "World Expansion")
    TArray<FEng_AssetPipeline> AssetPipelines;

    // Performance targets
    UPROPERTY(EditAnywhere, Category = "World Expansion")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, Category = "World Expansion")
    int32 MaxStreamingActors;

    UPROPERTY(EditAnywhere, Category = "World Expansion")
    float MaxMemoryUsageMB;

private:
    // Internal state
    bool bWorldPartitionConfigured;
    bool bBiomeDistributionSetup;
    bool bAssetPipelineReady;
    
    // Performance tracking
    float CurrentStreamingPerformance;
    TArray<FString> PerformanceWarnings;
    
    // Validation cache
    bool bValidationCacheValid;
    TArray<FString> CachedBlockers;
    FDateTime LastValidationTime;

    // Internal methods
    void InitializeDefaultBiomes();
    void SetupCommercialAssetPipelines();
    void ValidateWorldPartitionSetup();
    void UpdatePerformanceMetrics();
    bool CheckLandscapeCompatibility();
    bool ValidateStreamingSetup();
};