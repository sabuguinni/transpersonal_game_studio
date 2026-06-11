#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// Forward declarations
class UPCGWorldGenerator;
class UFoliageManager;

/**
 * Biome configuration data structure
 * Defines environmental parameters for each biome type
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfig
{
    GENERATED_BODY()

    FEng_BiomeConfig()
        : BiomeType(EBiomeType::Temperate)
        , Temperature(20.0f)
        , Humidity(0.5f)
        , Elevation(100.0f)
        , VegetationDensity(0.7f)
        , WaterLevel(0.3f)
        , RockDensity(0.4f)
        , SoilType(ESoilType::Loam)
        , WeatherIntensity(0.5f)
        , DangerLevel(0.3f)
    {
    }

    /** Primary biome classification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    /** Average temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "-40.0", ClampMax = "60.0"))
    float Temperature;

    /** Humidity level (0.0 = arid, 1.0 = saturated) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity;

    /** Base elevation above sea level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Elevation;

    /** Vegetation coverage density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flora", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity;

    /** Water presence level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterLevel;

    /** Rock and mineral density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RockDensity;

    /** Soil composition type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ESoilType SoilType;

    /** Weather pattern intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherIntensity;

    /** Predator/hazard danger level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerLevel;
};

/**
 * Biome transition zone data
 * Manages smooth transitions between different biomes
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    FEng_BiomeTransition()
        : FromBiome(EBiomeType::Temperate)
        , ToBiome(EBiomeType::Temperate)
        , TransitionWidth(500.0f)
        , BlendCurve(nullptr)
    {
    }

    /** Source biome type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome;

    /** Target biome type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome;

    /** Width of transition zone in UE units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "100.0"))
    float TransitionWidth;

    /** Curve defining blend falloff */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    class UCurveFloat* BlendCurve;
};

/**
 * Central biome management system
 * Handles biome classification, transitions, and environmental parameters
 * Integrates with PCG World Generator and Foliage Manager
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Get biome type at world location */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get complete biome configuration at location */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeConfig GetBiomeConfig(const FVector& WorldLocation) const;

    /** Get interpolated biome parameters considering transitions */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeConfig GetInterpolatedBiomeData(const FVector& WorldLocation) const;

    /** Register a biome configuration */
    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor)
    void RegisterBiomeConfig(EBiomeType BiomeType, const FEng_BiomeConfig& Config);

    /** Set biome for a specific region */
    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor)
    void SetBiomeRegion(const FVector& Center, float Radius, EBiomeType BiomeType);

    /** Generate biome map for world */
    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor)
    void GenerateBiomeMap();

    /** Validate biome system integrity */
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    bool ValidateBiomeSystem() const;

    /** Get all configured biome types */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EBiomeType> GetConfiguredBiomes() const;

    /** Calculate biome transition factor between two points */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float CalculateTransitionFactor(const FVector& LocationA, const FVector& LocationB) const;

protected:
    /** Biome configuration database */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<EBiomeType, FEng_BiomeConfig> BiomeConfigs;

    /** Biome transition rules */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    /** World grid resolution for biome mapping */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (ClampMin = "100.0"))
    float GridResolution;

    /** Maximum transition blend distance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (ClampMin = "100.0"))
    float MaxTransitionDistance;

    /** Enable debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bDebugVisualization;

private:
    /** Initialize default biome configurations */
    void InitializeDefaultBiomes();

    /** Calculate biome from world position using noise */
    EBiomeType CalculateBiomeFromNoise(const FVector& WorldLocation) const;

    /** Find transition configuration between biomes */
    const FEng_BiomeTransition* FindTransition(EBiomeType FromBiome, EBiomeType ToBiome) const;

    /** Interpolate between two biome configs */
    FEng_BiomeConfig InterpolateBiomeConfigs(const FEng_BiomeConfig& ConfigA, const FEng_BiomeConfig& ConfigB, float Alpha) const;

    /** Cached reference to world generator */
    UPROPERTY()
    UPCGWorldGenerator* WorldGenerator;

    /** Cached reference to foliage manager */
    UPROPERTY()
    UFoliageManager* FoliageManager;

    /** Runtime biome map cache */
    TMap<FIntPoint, EBiomeType> BiomeCache;

    /** Cache validity flag */
    bool bBiomeCacheValid;
};