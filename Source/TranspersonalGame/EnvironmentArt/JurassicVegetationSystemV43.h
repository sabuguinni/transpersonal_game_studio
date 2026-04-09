#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "FoliageType.h"
#include "InstancedFoliageActor.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "JurassicVegetationSystemV43.generated.h"

// Forward declarations
class UFoliageType_InstancedStaticMesh;
class UPCGComponent;
class UStaticMesh;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EJurassicVegetationType : uint8
{
    Cycads          UMETA(DisplayName = "Cycads"),
    TreeFerns       UMETA(DisplayName = "Tree Ferns"),
    Conifers        UMETA(DisplayName = "Conifers"),
    GinkgoTrees     UMETA(DisplayName = "Ginkgo Trees"),
    Horsetails      UMETA(DisplayName = "Horsetails"),
    Bennettitales   UMETA(DisplayName = "Bennettitales"),
    DeadWood        UMETA(DisplayName = "Dead Wood"),
    Undergrowth     UMETA(DisplayName = "Undergrowth")
};

UENUM(BlueprintType)
enum class EJurassicBiomeType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    VolcanicPlains  UMETA(DisplayName = "Volcanic Plains"),
    CoastalCliffs   UMETA(DisplayName = "Coastal Cliffs"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    Swamplands      UMETA(DisplayName = "Swamplands")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicVegetationSpec
{
    GENERATED_BODY()

    // Basic Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EJurassicVegetationType VegetationType = EJurassicVegetationType::TreeFerns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> VegetationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    // Size and Scaling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float MinScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float MaxScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float MinHeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float MaxHeight = 15.0f;

    // Placement Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float DensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float MaxSlope = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (ClampMin = "-500.0", ClampMax = "500.0"))
    float MinAltitude = -100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (ClampMin = "-500.0", ClampMax = "500.0"))
    float MaxAltitude = 300.0f;

    // Environmental Preferences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterProximityPreference = 0.5f; // 0 = avoid water, 1 = prefer water

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SunlightRequirement = 0.7f; // 0 = shade tolerant, 1 = full sun

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<EJurassicBiomeType> PreferredBiomes;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableNanite = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float CullDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "10000"))
    int32 MaxInstancesPerChunk = 500;

    // Storytelling Elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bCanHaveFossilizedVersion = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bCanShowDamage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AgeVariationChance = 0.3f;

    FJurassicVegetationSpec()
    {
        PreferredBiomes.Add(EJurassicBiomeType::DenseForest);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeVegetationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EJurassicBiomeType BiomeType = EJurassicBiomeType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FJurassicVegetationSpec> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "3.0"))
    float OverallDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VarietyFactor = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftObjectPtr<UPCGGraph> BiomePCGGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FLinearColor BiomeColorTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HumidityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TemperatureLevel = 0.7f; // Jurassic was generally warm
};

/**
 * Data Asset containing all Jurassic vegetation specifications and biome configurations
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UJurassicVegetationDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Database")
    TArray<FJurassicVegetationSpec> VegetationDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configurations")
    TArray<FBiomeVegetationConfig> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float GlobalDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bEnableSeasonalVariation = false; // Jurassic had stable climate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalVegetationInstances = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GlobalCullDistance = 3000.0f;

    // Helper functions
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    FJurassicVegetationSpec GetVegetationSpec(EJurassicVegetationType VegetationType) const;

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    FBiomeVegetationConfig GetBiomeConfig(EJurassicBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    TArray<FJurassicVegetationSpec> GetVegetationForBiome(EJurassicBiomeType BiomeType) const;
};

/**
 * Main Jurassic Vegetation System Component
 * Manages procedural placement and rendering of prehistoric vegetation
 */
UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UJurassicVegetationSystemV43 : public UActorComponent
{
    GENERATED_BODY()

public:
    UJurassicVegetationSystemV43();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation System")
    TSoftObjectPtr<UJurassicVegetationDataAsset> VegetationDataAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation System")
    bool bAutoGenerateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation System")
    bool bUseWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float GenerationRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float ChunkSize = 200.0f;

    // PCG Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGComponent> PCGComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bUsePCGForGeneration = true;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "30", ClampMax = "120"))
    int32 TargetFPS = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableGPUCulling = true;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentVegetationCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<EJurassicBiomeType> ActiveBiomes;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void GenerateVegetationInArea(FVector Center, float Radius, EJurassicBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void ClearVegetationInArea(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void RegenerateAllVegetation();

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void SetBiomeType(EJurassicBiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    void UpdatePerformanceSettings(int32 NewTargetFPS, bool bNewEnableGPUCulling);

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    FVector GetOptimalSpawnLocation(FVector DesiredLocation, EJurassicVegetationType VegetationType) const;

    UFUNCTION(BlueprintCallable, Category = "Vegetation System")
    bool CanPlaceVegetationAt(FVector Location, EJurassicVegetationType VegetationType) const;

    // Environmental Storytelling
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void AddStorytellingElement(FVector Location, const FString& StoryElement);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateFossilizedVegetation(FVector Location, EJurassicVegetationType OriginalType);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void AddEnvironmentalDamage(FVector Location, float Radius, const FString& DamageType);

private:
    // Internal Systems
    void InitializeVegetationSystem();
    void SetupPCGIntegration();
    void ConfigureFoliageTypes();
    void UpdateLODSystem();
    void ProcessPerformanceOptimization();
    
    // Generation Helpers
    void GenerateVegetationChunk(FVector ChunkCenter, EJurassicBiomeType BiomeType);
    bool ValidatePlacementLocation(FVector Location, const FJurassicVegetationSpec& VegSpec) const;
    FTransform CalculateVegetationTransform(FVector Location, const FJurassicVegetationSpec& VegSpec) const;
    
    // Performance Monitoring
    void MonitorPerformance();
    void AdjustQualitySettings();
    
    // Cache
    UPROPERTY()
    TMap<EJurassicBiomeType, FBiomeVegetationConfig> CachedBiomeConfigs;
    
    UPROPERTY()
    TMap<EJurassicVegetationType, FJurassicVegetationSpec> CachedVegetationSpecs;
    
    // Runtime Data
    float LastPerformanceCheck = 0.0f;
    float CurrentFrameTime = 0.0f;
    bool bPerformanceOptimizationActive = false;
};