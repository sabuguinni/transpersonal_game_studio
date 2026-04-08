#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "WorldGenerationManager.generated.h"

class UPCGComponent;
class ALandscape;
class AWaterBody;
class UWorldPartitionSubsystem;

/**
 * Biome types for the prehistoric world
 */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    DenseJungle         UMETA(DisplayName = "Dense Jungle"),
    OpenPlains          UMETA(DisplayName = "Open Plains"),
    RiverDelta          UMETA(DisplayName = "River Delta"),
    RockyOutcrops       UMETA(DisplayName = "Rocky Outcrops"),
    Swamplands          UMETA(DisplayName = "Swamplands"),
    CoastalRegion       UMETA(DisplayName = "Coastal Region")
};

/**
 * Terrain density levels for performance optimization
 */
UENUM(BlueprintType)
enum class ETerrainDensity : uint8
{
    Low                 UMETA(DisplayName = "Low Density"),
    Medium              UMETA(DisplayName = "Medium Density"),
    High                UMETA(DisplayName = "High Density"),
    Ultra               UMETA(DisplayName = "Ultra Density")
};

/**
 * Biome configuration data asset
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBiomeDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float TerrainRoughness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ElevationVariance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bHasWaterFeatures = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float WaterCoverage = 0.0f;
};

/**
 * World generation configuration
 */
USTRUCT(BlueprintType)
struct FWorldGenerationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    int32 WorldSizeX = 8129;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Size")
    int32 WorldSizeY = 8129;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    ETerrainDensity TerrainDensity = ETerrainDensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxComponentsPerGrid = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<UBiomeDataAsset*> AvailableBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    int32 MaxRiverSystems = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    float RiverWidth = 500.0f;
};

/**
 * Main world generation manager component
 * Handles procedural generation of terrain, biomes, and water systems
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorldGenerationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorldGenerationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // World Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRiverSystems();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateVegetation();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetTerrainHeightAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool IsLocationNearWater(FVector WorldLocation, float SearchRadius = 1000.0f);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveComponentCount();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetGenerationProgress();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FWorldGenerationConfig WorldConfig;

    // PCG Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> TerrainPCGComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> BiomePCGComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> VegetationPCGComponent;

    // Landscape Reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape")
    TObjectPtr<ALandscape> GeneratedLandscape;

    // Water System References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water")
    TArray<TObjectPtr<AWaterBody>> GeneratedWaterBodies;

    // Generation State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsGenerating;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float GenerationProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 CurrentGenerationStep;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 TotalGenerationSteps;

private:
    // Internal generation functions
    void InitializePCGComponents();
    void SetupWorldPartition();
    void CreateLandscapeActor();
    void ApplyBiomeDistribution();
    void GenerateRiverSplines();
    void PopulateVegetation();
    void OptimizeForPerformance();

    // Performance tracking
    int32 ActiveComponentCount;
    float LastPerformanceCheck;
    
    // World Partition
    UWorldPartitionSubsystem* WorldPartitionSubsystem;
};