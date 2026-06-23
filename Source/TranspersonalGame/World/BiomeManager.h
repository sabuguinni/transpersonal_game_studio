#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "TranspersonalGame/SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// Biome transition data — defines how two biomes blend
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType FromBiome = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType ToBiome = EBiomeType::Forest;

    /** Width of the transition zone in world units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float TransitionWidth = 1000.0f;

    /** Blend curve exponent (1=linear, 2=quadratic) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.5", ClampMax = "4.0"))
    float BlendExponent = 1.5f;
};

// ============================================================
// Per-biome runtime data snapshot
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeRuntimeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Grassland;

    /** Normalised blend weight at the query point [0..1] */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendWeight = 1.0f;

    /** Ambient temperature in Celsius */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Temperature = 28.0f;

    /** Relative humidity [0..1] */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Humidity = 0.5f;

    /** Vegetation density scalar [0..1] */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float VegetationDensity = 0.7f;

    /** Dominant dinosaur species in this biome */
    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EDinosaurSpecies DominantSpecies = EDinosaurSpecies::Raptor;
};

// ============================================================
// ABiomeManager — world actor that owns the biome grid
// ============================================================
UCLASS(ClassGroup = "TranspersonalGame|World", meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ---- AActor overrides ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Public API (callable from Blueprint / other C++) ----

    /**
     * Returns the dominant biome type at a given world location.
     * Uses a fast noise-based lookup — safe to call every frame.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns full runtime data (blend weights, temperature, humidity…)
     * for the biome at the given world location.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeRuntimeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns the current weather state for the biome at the given location.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWeatherType GetWeatherAtLocation(const FVector& WorldLocation) const;

    /**
     * Forces a weather override on a specific biome (useful for scripted events).
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SetWeatherOverride(EBiomeType Biome, EWeatherType Weather, float DurationSeconds);

    /**
     * Returns true if the two world positions are in the same biome.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool AreSameBiome(const FVector& LocationA, const FVector& LocationB) const;

    /**
     * Debug: draws biome boundaries in the viewport for N seconds.
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DrawBiomeBoundaries(float Duration = 10.0f);

    // ---- Configuration ----

    /** Total world size in X/Y (should match landscape) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldSizeXY = 16000.0f;

    /** Noise frequency for biome distribution (lower = larger biomes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config", meta = (ClampMin = "0.00001", ClampMax = "0.001"))
    float BiomeNoiseFrequency = 0.0001f;

    /** Noise seed — change to get a different world layout */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    int32 NoiseSeed = 42;

    /** Transition definitions between adjacent biomes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    /** How often (seconds) the weather system ticks per biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Weather", meta = (ClampMin = "1.0", ClampMax = "300.0"))
    float WeatherTickInterval = 30.0f;

private:
    // Internal noise helpers
    float SampleNoise(float X, float Y, int32 Seed) const;
    EBiomeType ClassifyBiome(float NoiseValue, float Altitude) const;
    float GetAltitudeAtLocation(const FVector& WorldLocation) const;

    // Weather state per biome
    TMap<EBiomeType, EWeatherType> CurrentWeatherMap;
    TMap<EBiomeType, float>        WeatherOverrideTimers;

    // Cached world reference
    float WeatherAccumulator = 0.0f;

    void TickWeather(float DeltaTime);
    void InitialiseDefaultWeather();
    void InitialiseDefaultTransitions();
};
