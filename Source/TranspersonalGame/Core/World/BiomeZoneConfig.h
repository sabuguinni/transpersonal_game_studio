// BiomeZoneConfig.h
// Agent #05 — Procedural World Generator
// Biome zone configuration for the Cretaceous prehistoric world
// Defines biome types, terrain parameters, and vegetation density

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataAsset.h"
#include "BiomeZoneConfig.generated.h"

// Biome types for the Cretaceous prehistoric world
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    TropicalJungle     UMETA(DisplayName = "Tropical Jungle"),
    OpenSavanna        UMETA(DisplayName = "Open Savanna"),
    RockyHighlands     UMETA(DisplayName = "Rocky Highlands"),
    RiverDelta         UMETA(DisplayName = "River Delta"),
    VolcanicPlains     UMETA(DisplayName = "Volcanic Plains"),
    CoastalShallows    UMETA(DisplayName = "Coastal Shallows"),
    FernMeadow         UMETA(DisplayName = "Fern Meadow"),
    DenseConifer       UMETA(DisplayName = "Dense Conifer Forest")
};

// Terrain noise parameters for height variation
USTRUCT(BlueprintType)
struct FWorld_TerrainNoiseParams
{
    GENERATED_BODY()

    // Base frequency of the noise (lower = larger features)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    float Frequency = 0.0003f;

    // Amplitude multiplier for height variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    float Amplitude = 1200.0f;

    // Number of octaves for fractal noise
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    int32 Octaves = 6;

    // Persistence (how much each octave contributes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    float Persistence = 0.5f;

    // Lacunarity (frequency multiplier per octave)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    float Lacunarity = 2.0f;

    // Seed for reproducible generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Noise")
    int32 Seed = 42;
};

// Vegetation density config per biome
USTRUCT(BlueprintType)
struct FWorld_VegetationDensity
{
    GENERATED_BODY()

    // Trees per 10000 square units
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float TreeDensity = 0.5f;

    // Ferns/undergrowth per 10000 square units
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float UndergrowthDensity = 2.0f;

    // Large rocks per 10000 square units
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float RockDensity = 0.2f;

    // Whether water bodies can spawn in this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bAllowWaterBodies = false;

    // Minimum tree height scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float MinTreeScale = 0.8f;

    // Maximum tree height scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float MaxTreeScale = 3.5f;
};

// Weather parameters per biome
USTRUCT(BlueprintType)
struct FWorld_BiomeWeather
{
    GENERATED_BODY()

    // Base temperature in Celsius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float BaseTemperature = 28.0f;

    // Humidity 0-1 (affects fog density and rain frequency)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity = 0.7f;

    // Rain probability 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainProbability = 0.3f;

    // Storm probability 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float StormProbability = 0.05f;

    // Fog density multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier = 1.0f;

    // Wind speed in km/h
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed = 15.0f;
};

// Complete biome zone definition
USTRUCT(BlueprintType)
struct FWorld_BiomeZoneData
{
    GENERATED_BODY()

    // Biome type identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::TropicalJungle;

    // Display name for UI/debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Tropical Jungle");

    // Center location of this biome zone in world space
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Location")
    FVector ZoneCenter = FVector::ZeroVector;

    // Radius of the biome zone in world units
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Location")
    float ZoneRadius = 5000.0f;

    // Blend distance at zone edges (smooth transition)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Location")
    float BlendDistance = 800.0f;

    // Terrain noise parameters for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    FWorld_TerrainNoiseParams TerrainNoise;

    // Vegetation density settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation")
    FWorld_VegetationDensity Vegetation;

    // Weather parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Weather")
    FWorld_BiomeWeather Weather;

    // Dominant dinosaur species that inhabit this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Fauna")
    TArray<FString> InhabitingDinoSpecies;

    // Ambient audio cue path for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    FSoftObjectPath AmbientAudioCue;

    // Ground material path for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FSoftObjectPath GroundMaterial;

    // Fog color tint for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor FogColorTint = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f);
};

// Data asset holding all biome zone configurations for the world
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UWorld_BiomeZoneConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UWorld_BiomeZoneConfig();

    // All biome zones in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZoneData> BiomeZones;

    // Global world seed for reproducible generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 WorldSeed = 12345;

    // Total world size in world units (square)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float WorldSize = 40000.0f;

    // Sea level height (Z coordinate)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float SeaLevel = 0.0f;

    // Maximum terrain height above sea level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float MaxTerrainHeight = 3000.0f;

    // Get biome data at a given world location
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FWorld_BiomeZoneData GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Get the dominant biome type at a world location
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    EWorld_BiomeType GetBiomeTypeAtLocation(const FVector& WorldLocation) const;

    // Get blend weight for a specific biome at a location (0-1)
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    float GetBiomeBlendWeight(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    // Initialize default Cretaceous world biome layout
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Setup")
    void InitializeDefaultCretaceousLayout();

    // Validate all biome zones for overlaps and coverage
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Validation")
    bool ValidateBiomeLayout() const;
};
