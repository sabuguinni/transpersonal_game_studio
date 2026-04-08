#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Landscape/Classes/Landscape.h"
#include "ProceduralWorldManager.generated.h"

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    DenseJungle         UMETA(DisplayName = "Dense Jungle"),
    OpenPlains          UMETA(DisplayName = "Open Plains"),
    RiverValley         UMETA(DisplayName = "River Valley"),
    RockyOutcrops       UMETA(DisplayName = "Rocky Outcrops"),
    Swampland          UMETA(DisplayName = "Swampland"),
    CoastalArea        UMETA(DisplayName = "Coastal Area")
};

UENUM(BlueprintType)
enum class EDangerLevel : uint8
{
    Safe               UMETA(DisplayName = "Safe - Herbivore Territory"),
    Moderate           UMETA(DisplayName = "Moderate - Mixed Territory"),
    High               UMETA(DisplayName = "High - Predator Territory"),
    Extreme            UMETA(DisplayName = "Extreme - Apex Predator Territory")
};

USTRUCT(BlueprintType)
struct FBiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::DenseJungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EDangerLevel DangerLevel = EDangerLevel::Moderate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterPresence = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerrainRoughness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D HeightRange = FVector2D(-500.0f, 2000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UMaterialInterface>> TerrainMaterials;
};

USTRUCT(BlueprintType)
struct FWorldGenSettings
{
    GENERATED_BODY()

    // World Partition Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 CellSize = 25600; // 256m cells for optimal streaming

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 LoadingRange = 76800; // 768m loading range

    // PCG Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bUseHierarchicalGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bUseRuntimeGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bUseGPUProcessing = true;

    // Landscape Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FIntPoint LandscapeSize = FIntPoint(2017, 2017); // Recommended size

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 QuadsPerSection = 63;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 SectionsPerComponent = 4; // 2x2 sections

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    float ZScale = 100.0f;

    // Biome Distribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FBiomeConfiguration> BiomeConfigurations;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxConcurrentGenerations = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTimeAllocation = 16.667f; // 60fps target
};

/**
 * Central manager for procedural world generation
 * Coordinates PCG, World Partition, and Landscape systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProceduralWorldManager : public AActor
{
    GENERATED_BODY()

public:
    AProceduralWorldManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPCGComponent* MasterPCGComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DebugVisualizationComponent;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    FWorldGenSettings WorldSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TSoftObjectPtr<UPCGGraph> MasterWorldGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TSoftObjectPtr<ALandscape> TargetLandscape;

    // Generation Control
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRegion(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CleanupRegion(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void RegenerateAllBiomes();

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EDangerLevel GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FBiomeConfiguration GetBiomeConfiguration(EBiomeType BiomeType) const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleDebugVisualization();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowBiomeMap();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowDangerZones();

protected:
    // Internal generation methods
    void SetupWorldPartition();
    void ConfigurePCGSystem();
    void InitializeBiomeDistribution();
    void CreateLandscapeIfNeeded();

    // Biome calculation
    FVector2D WorldToNormalizedCoordinates(const FVector& WorldLocation) const;
    float CalculateNoiseValue(const FVector2D& Coordinates, float Scale, int32 Octaves = 4) const;

private:
    // Runtime state
    bool bIsInitialized = false;
    bool bDebugVisualizationEnabled = false;
    
    // Cached biome data for performance
    TMap<FIntPoint, EBiomeType> BiomeCache;
    TMap<FIntPoint, EDangerLevel> DangerCache;
    
    // Performance tracking
    float LastGenerationTime = 0.0f;
    int32 ActiveGenerations = 0;
};