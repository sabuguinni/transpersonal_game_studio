// BiomeManager.h
// Agent #05 — Procedural World Generator
// Manages 5 distinct biomes: Forest, Plains, Rocky Highlands, Wetlands, Volcanic Badlands
// P1 Priority: World Generation

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// Biome type enum — unique prefix World_ to avoid conflicts
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest          UMETA(DisplayName = "Dense Fern Forest"),
    Plains          UMETA(DisplayName = "Open Plains"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    Wetlands        UMETA(DisplayName = "River Wetlands"),
    VolcanicBadlands UMETA(DisplayName = "Volcanic Badlands"),
    None            UMETA(DisplayName = "None / Transition")
};

// Per-biome configuration data
USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseElevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationVariance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;
};

// Runtime biome query result
USTRUCT(BlueprintType)
struct FWorld_BiomeQueryResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType PrimaryBiome = EWorld_BiomeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType SecondaryBiome = EWorld_BiomeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendFactor = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float DangerLevel = 0.5f;
};

/**
 * ABiomeManager
 * Placed once in the level. Defines 5 biome zones and provides
 * spatial queries for any world position → biome data.
 * Used by: FoliageManager, DinosaurAI, WeatherSystem, AudioSystem
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "WorldGen")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // === BIOME CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float TransitionBlendRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    bool bDebugDrawBiomeBounds = false;

    // === SPATIAL QUERY API ===

    /** Get biome data at a world position (with blend between adjacent biomes) */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeQueryResult QueryBiomeAtLocation(const FVector& WorldLocation) const;

    /** Get the dominant biome type at a world position */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeTypeAtLocation(const FVector& WorldLocation) const;

    /** Get temperature at location (affected by biome + elevation) */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Get humidity at location */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /** Get danger level at location (affects AI spawning) */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    /** Check if a location is in a water biome (wetlands/river) */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool IsWaterBiome(const FVector& WorldLocation) const;

    /** Get config for a specific biome type */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool GetBiomeConfig(EWorld_BiomeType BiomeType, FWorld_BiomeConfig& OutConfig) const;

    /** Initialize default 5-biome layout matching MinPlayableMap */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biomes")
    void InitializeDefaultBiomes();

    // === FOLIAGE DENSITY ===
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetFoliageDensityAtLocation(const FVector& WorldLocation) const;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    /** Find the nearest biome config to a world location */
    int32 FindNearestBiomeIndex(const FVector& WorldLocation) const;

    /** Find the two nearest biomes for blending */
    void FindTwoNearestBiomes(const FVector& WorldLocation, int32& OutFirst, int32& OutSecond, float& OutBlend) const;
};
