#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGVolume.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "JurassicWorldGeneratorV43.generated.h"

UENUM(BlueprintType)
enum class EWorld_JurassicBiomeType_1A8 : uint8
{
    DenseForest      UMETA(DisplayName = "Dense Forest"),
    RiverDelta       UMETA(DisplayName = "River Delta"), 
    VolcanicPlains   UMETA(DisplayName = "Volcanic Plains"),
    CoastalCliffs    UMETA(DisplayName = "Coastal Cliffs"),
    OpenPlains       UMETA(DisplayName = "Open Plains"),
    SwampLands       UMETA(DisplayName = "Swamp Lands"),
    MountainousRegion UMETA(DisplayName = "Mountainous Region")
};

UENUM(BlueprintType)
enum class EWorldGenerationPhase : uint8
{
    Initialization   UMETA(DisplayName = "Initialization"),
    TerrainGeneration UMETA(DisplayName = "Terrain Generation"),
    BiomeDistribution UMETA(DisplayName = "Biome Distribution"),
    WaterSystems     UMETA(DisplayName = "Water Systems"),
    VegetationPlacement UMETA(DisplayName = "Vegetation Placement"),
    GeologicalFeatures UMETA(DisplayName = "Geological Features"),
    Finalization     UMETA(DisplayName = "Finalization")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_JurassicBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_JurassicBiomeType_1A8 BiomeType = EWorld_JurassicBiomeType_1A8::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeInfluence = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HeightVariation = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterPresence = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> BiomeSpecificMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftObjectPtr<UMaterialInterface> BiomeMaterial;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverSystemData_1A8
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FString RiverName = TEXT("UnnamedRiver");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    TArray<FVector> RiverSplinePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float RiverWidth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float RiverDepth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed = 200.0f;

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    TArray<FWorld_RiverSystemData_1A8> Tributaries;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorldGenerationSettings
{
    GENERATED_BODY()

    // World Scale Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scale")
    float WorldSize = 16384.0f; // 16km x 16km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scale")
    float HeightmapResolution = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scale")
    float MaxWorldHeight = 2000.0f;

    // PCG Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    int32 PCGGridSize = 512; // 512m cells

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bUseHierarchicalGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bEnableRuntimeGeneration = true;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveChunks = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LODLevels = 4;
};

/**
 * Advanced Jurassic World Generator V43
 * 
 * Generates vast, procedural Jurassic landscapes using UE5's PCG Framework,
 * World Partition, and advanced biome systems. Creates realistic prehistoric
 * environments with proper geological features, river systems, and biome transitions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AJurassicWorldGeneratorV43 : public AActor
{
    GENERATED_BODY()

public:
    AJurassicWorldGeneratorV43();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime);

    // Core Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateJurassicWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrainBase();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void DistributeBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateRiverSystems();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PlaceGeologicalFeatures();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void FinalizeWorldGeneration();

    // PCG Integration
    UFUNCTION(BlueprintCallable, Category = "PCG")
    void SetupPCGSystem();

    UFUNCTION(BlueprintCallable, Category = "PCG")
    void CreatePCGVolumes();

    UFUNCTION(BlueprintCallable, Category = "PCG")
    void ConfigurePCGGraphs();

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void CreateBiome(EWorld_JurassicBiomeType_1A8 BiomeType, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_JurassicBiomeData GetBiomeDataAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<FWorld_JurassicBiomeData> GetAllBiomes() const { return BiomeData; }

    // River System
    UFUNCTION(BlueprintCallable, Category = "Rivers")
    void CreateMainRiverSystem();

    UFUNCTION(BlueprintCallable, Category = "Rivers")
    void CreateTributary(const FWorld_RiverSystemData_1A8& TributaryData);

    // World Partition Integration
    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void SetupWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void ConfigureStreamingSources();

    // Performance & Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetupLODSystem();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetHeightAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FVector GetNormalAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool IsLocationInWater(FVector WorldLocation);

    // Debug & Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawBiomes();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawRiverSystems();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleDebugVisualization();

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPCGComponent* PCGComponent;

    // Generation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FWorldGenerationSettings GenerationSettings;

    // Biome System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_JurassicBiomeData> BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EWorld_JurassicBiomeType_1A8, TSoftObjectPtr<UPCGGraph>> BiomePCGGraphs;

    // River System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    FWorld_RiverSystemData_1A8 MainRiverSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    TArray<FWorld_RiverSystemData_1A8> AdditionalRivers;

    // World References
    UPROPERTY()
    class ALandscape* WorldLandscape;

    UPROPERTY()
    class APCGWorldActor* PCGWorldActor;

    UPROPERTY()
    class UWorldPartitionSubsystem* WorldPartitionSubsystem;

    // Generation State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EWorldGenerationPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsGenerating;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float GenerationProgress;

    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePCGComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 GeneratedActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastGenerationTime;

    // Debug Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawBiomeBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawRiverSplines;

private:
    // Internal Helper Functions
    void InitializePCGSystem();
    void CreateBiomeTransitions();
    void GenerateHeightmapNoise();
    void ApplyBiomeInfluence(FVector Location, float Radius, float Strength);
    void ValidateWorldGeneration();
    
    // Performance Monitoring
    void UpdatePerformanceMetrics();
    void LogGenerationStats();

    // Async Generation Support
    void StartAsyncGeneration();
    void OnAsyncGenerationComplete();

    FTimerHandle GenerationTimerHandle;
    FTimerHandle PerformanceTimerHandle;
};