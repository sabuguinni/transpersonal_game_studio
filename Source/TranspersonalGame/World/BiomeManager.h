#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * Biome types for the prehistoric world.
 * Each biome has distinct terrain, vegetation, fauna, and weather characteristics.
 */
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Dense Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    River       UMETA(DisplayName = "River Delta"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic Region"),
    Count       UMETA(Hidden)
};

/**
 * Per-biome configuration data.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    /** World-space center of this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    /** Radius of influence in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 250000.f;

    /** Ambient temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 28.f;

    /** Humidity 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    /** Vegetation density 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    /** Danger level 0-1 (affects predator spawn rate) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.3f;

    /** Fog density override for this biome (0 = use global) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensityOverride = 0.f;

    FWorld_BiomeConfig() = default;
};

/**
 * ABiomeManager — Manages biome zones in the procedural prehistoric world.
 *
 * Placed once in the level. Defines biome boundaries, queries biome at any
 * world position, and drives environmental parameters (temperature, humidity,
 * danger) for survival systems.
 *
 * Agent #5 — Procedural World Generator
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "WorldGen")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // --- Biome Configuration ---

    /** All biome zones in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeZones;

    /** Blend radius between biomes in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeBlendRadius = 50000.f;

    // --- Runtime Queries ---

    /**
     * Returns the dominant biome type at a given world location.
     * Uses distance-weighted blending between overlapping biomes.
     */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns the full biome config for the dominant biome at a location.
     * Returns false if no biome covers the location.
     */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool GetBiomeConfigAtLocation(const FVector& WorldLocation, FWorld_BiomeConfig& OutConfig) const;

    /**
     * Returns interpolated temperature at a world location (blended across biomes).
     */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns interpolated humidity at a world location.
     */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns danger level at a world location (used by AI spawn system).
     */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns all biomes within a given radius of a location.
     */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<EWorld_BiomeType> GetNearbyBiomes(const FVector& WorldLocation, float SearchRadius) const;

    // --- Editor Utilities ---

    /** Populate default biome zones matching the MinPlayableMap layout */
    UFUNCTION(CallInEditor, Category = "Biomes")
    void SetupDefaultBiomes();

    /** Log all biome zones to output log */
    UFUNCTION(CallInEditor, Category = "Biomes")
    void DebugPrintBiomes() const;

protected:
    virtual void BeginPlay() override;

private:
    /** Compute normalized influence weight of a biome at a location */
    float ComputeBiomeWeight(const FWorld_BiomeConfig& Biome, const FVector& Location) const;
};
