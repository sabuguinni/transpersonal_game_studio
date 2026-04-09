#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataAsset.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "TerrainGenerator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTerrainGenerator, Log, All);

/**
 * Noise Types for terrain generation
 */
UENUM(BlueprintType)
enum class ENoiseType : uint8
{
    Perlin              UMETA(DisplayName = "Perlin Noise"),
    Simplex             UMETA(DisplayName = "Simplex Noise"),
    Ridged              UMETA(DisplayName = "Ridged Noise"),
    Voronoi             UMETA(DisplayName = "Voronoi Noise"),
    Billowy             UMETA(DisplayName = "Billowy Noise"),
    Cellular            UMETA(DisplayName = "Cellular Noise")
};

/**
 * Erosion Types
 */
UENUM(BlueprintType)
enum class EErosionType : uint8
{
    Hydraulic           UMETA(DisplayName = "Hydraulic Erosion"),
    Thermal             UMETA(DisplayName = "Thermal Erosion"),
    Chemical            UMETA(DisplayName = "Chemical Erosion"),
    Glacial             UMETA(DisplayName = "Glacial Erosion")
};

/**
 * Noise Layer Configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNoiseLayer
{
    GENERATED_BODY()

    /** Type of noise to generate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    ENoiseType NoiseType = ENoiseType::Perlin;

    /** Frequency/scale of the noise */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0.001", ClampMax = "10.0"))
    float Frequency = 0.01f;

    /** Amplitude/strength of the noise */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Amplitude = 1.0f;

    /** Number of octaves for fractal noise */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "1", ClampMax = "8"))
    int32 Octaves = 4;

    /** Lacunarity for fractal noise */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "1.0", ClampMax = "4.0"))
    float Lacunarity = 2.0f;

    /** Persistence for fractal noise */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Persistence = 0.5f;

    /** Offset for the noise */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    FVector2D Offset = FVector2D::ZeroVector;

    /** Enable this noise layer */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    bool bEnabled = true;

    FNoiseLayer()
    {
        NoiseType = ENoiseType::Perlin;
        Frequency = 0.01f;
        Amplitude = 1.0f;
        Octaves = 4;
        Lacunarity = 2.0f;
        Persistence = 0.5f;
        Offset = FVector2D::ZeroVector;
        bEnabled = true;
    }
};

/**
 * Erosion Parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FErosionParams
{
    GENERATED_BODY()

    /** Type of erosion to apply */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    EErosionType ErosionType = EErosionType::Hydraulic;

    /** Strength of erosion */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Strength = 0.3f;

    /** Number of iterations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "1", ClampMax = "100"))
    int32 Iterations = 10;

    /** Rain amount for hydraulic erosion */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RainAmount = 0.1f;

    /** Evaporation rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EvaporationRate = 0.02f;

    /** Sediment capacity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hydraulic", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float SedimentCapacity = 4.0f;

    /** Thermal erosion angle */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float ThermalAngle = 30.0f;

    FErosionParams()
    {
        ErosionType = EErosionType::Hydraulic;
        Strength = 0.3f;
        Iterations = 10;
        RainAmount = 0.1f;
        EvaporationRate = 0.02f;
        SedimentCapacity = 4.0f;
        ThermalAngle = 30.0f;
    }
};

/**
 * Terrain Generation Profile
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UTerrainProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Base elevation in meters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    float BaseElevation = 0.0f;

    /** Maximum elevation in meters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    float MaxElevation = 2000.0f;

    /** Noise layers for terrain generation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    TArray<FNoiseLayer> NoiseLayers;

    /** Erosion parameters */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Erosion")
    TArray<FErosionParams> ErosionSteps;

    /** Terrain material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    /** Landscape physical material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    TSoftObjectPtr<UPhysicalMaterial> LandscapePhysicalMaterial;
};

/**
 * Terrain Generator Component
 * Handles procedural terrain generation using advanced noise and erosion algorithms
 */
UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTerrainGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UTerrainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    /** Generate heightmap data */
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateHeightmap(TArray<uint16>& OutHeightData, int32 Resolution, int32 Seed = 12345);

    /** Generate terrain using profile */
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainFromProfile(UTerrainProfile* Profile, int32 Resolution, int32 Seed = 12345);

    /** Apply noise layer to heightmap */
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ApplyNoiseLayer(TArray<float>& HeightData, const FNoiseLayer& NoiseLayer, int32 Resolution, int32 Seed);

    /** Apply erosion to heightmap */
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ApplyErosion(TArray<float>& HeightData, const FErosionParams& ErosionParams, int32 Resolution);

    /** Generate Perlin noise */
    UFUNCTION(BlueprintPure, Category = "Noise")
    static float GeneratePerlinNoise(float X, float Y, int32 Seed = 0);

    /** Generate Simplex noise */
    UFUNCTION(BlueprintPure, Category = "Noise")
    static float GenerateSimplexNoise(float X, float Y, int32 Seed = 0);

    /** Generate Ridged noise */
    UFUNCTION(BlueprintPure, Category = "Noise")
    static float GenerateRidgedNoise(float X, float Y, int32 Seed = 0);

    /** Generate Voronoi noise */
    UFUNCTION(BlueprintPure, Category = "Noise")
    static float GenerateVoronoiNoise(float X, float Y, int32 Seed = 0);

    /** Generate fractal noise */
    UFUNCTION(BlueprintPure, Category = "Noise")
    static float GenerateFractalNoise(float X, float Y, int32 Octaves, float Lacunarity, float Persistence, int32 Seed = 0);

    /** Apply hydraulic erosion */
    UFUNCTION(BlueprintCallable, Category = "Erosion")
    void ApplyHydraulicErosion(TArray<float>& HeightData, int32 Resolution, const FErosionParams& Params);

    /** Apply thermal erosion */
    UFUNCTION(BlueprintCallable, Category = "Erosion")
    void ApplyThermalErosion(TArray<float>& HeightData, int32 Resolution, const FErosionParams& Params);

    /** Create landscape from heightmap */
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    ALandscape* CreateLandscape(const TArray<uint16>& HeightData, int32 Resolution, const FVector& Location, const FVector& Scale);

    /** Get height at world position */
    UFUNCTION(BlueprintPure, Category = "Terrain Query")
    float GetHeightAtPosition(const FVector& WorldPosition) const;

    /** Get slope at world position */
    UFUNCTION(BlueprintPure, Category = "Terrain Query")
    float GetSlopeAtPosition(const FVector& WorldPosition) const;

    /** Get normal at world position */
    UFUNCTION(BlueprintPure, Category = "Terrain Query")
    FVector GetNormalAtPosition(const FVector& WorldPosition) const;

protected:
    /** Current terrain profile */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TObjectPtr<UTerrainProfile> TerrainProfile;

    /** Generated landscape reference */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TObjectPtr<ALandscape> GeneratedLandscape;

    /** Cached heightmap data */
    UPROPERTY()
    TArray<float> CachedHeightData;

    /** Heightmap resolution */
    UPROPERTY()
    int32 HeightmapResolution = 0;

    /** World bounds for terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FVector2D WorldBounds = FVector2D(20000.0f, 20000.0f);

private:
    // Internal noise generation functions
    static float PerlinNoise2D(float X, float Y, int32 Seed);
    static float SimplexNoise2D(float X, float Y, int32 Seed);
    static float RidgedNoise2D(float X, float Y, int32 Seed);
    static float VoronoiNoise2D(float X, float Y, int32 Seed);
    
    // Utility functions
    static float Fade(float t);
    static float Lerp(float a, float b, float t);
    static float Grad(int32 Hash, float X, float Y);
    static int32 FastFloor(float f);
    
    // Erosion helper functions
    void SimulateWaterDroplet(TArray<float>& HeightData, int32 Resolution, const FErosionParams& Params, FVector2D StartPos);
    float GetHeightBilinear(const TArray<float>& HeightData, int32 Resolution, float X, float Y) const;
    FVector2D GetGradientBilinear(const TArray<float>& HeightData, int32 Resolution, float X, float Y) const;
    
    // Coordinate conversion
    FVector2D WorldToHeightmapCoords(const FVector& WorldPos) const;
    FVector HeightmapToWorldCoords(const FVector2D& HeightmapCoords, float Height) const;
    
    // Random number generation
    static FRandomStream RandomStream;
};