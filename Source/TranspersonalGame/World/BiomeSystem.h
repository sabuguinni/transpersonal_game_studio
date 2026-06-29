#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

// ============================================================
// Biome System — Agent #05 Procedural World Generator
// Three prehistoric biomes: Jungle, Plains, Volcanic
// Each biome defines vegetation density, ambient temperature,
// danger level, and PCG spawn rules.
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle    UMETA(DisplayName = "Jungle"),
    Plains    UMETA(DisplayName = "Plains"),
    Volcanic  UMETA(DisplayName = "Volcanic"),
    River     UMETA(DisplayName = "River"),
    None      UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    /** Center of this biome zone in world space */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    /** Radius of influence in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 300000.0f;

    /** Vegetation spawn density (0.0 = none, 1.0 = maximum) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity = 0.5f;

    /** Ambient temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 25.0f;

    /** Danger level — affects dinosaur spawn rate (0=safe, 1=lethal) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerLevel = 0.3f;

    /** Fog density multiplier for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensityMultiplier = 1.0f;

    /** Ambient sound tag for this biome (used by Audio Agent #16) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FName AmbientSoundTag = NAME_None;

    /** Water presence (river, lake, swamp) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;
};

USTRUCT(BlueprintType)
struct FWorld_PCGSpawnRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    EWorld_BiomeType TargetBiome = EWorld_BiomeType::None;

    /** Asset soft reference path for the mesh to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    FSoftObjectPath MeshPath;

    /** Spawn probability per PCG grid cell (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.1f;

    /** Min/Max scale range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    FVector2D ScaleRange = FVector2D(0.8f, 1.4f);

    /** Cull distance in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    float CullDistance = 8000.0f;

    /** Cast shadows — disabled for vegetation by default (performance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    bool bCastShadows = false;
};

/**
 * ABiomeManager — World actor that defines biome zones and PCG rules.
 * Placed once in the level. Provides biome query API for other systems.
 * Agent #06 (Environment Artist) reads this to place vegetation.
 * Agent #12 (Combat AI) reads DangerLevel for dinosaur spawning.
 * Agent #16 (Audio) reads AmbientSoundTag for biome audio.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    /** All biome zones in this world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeZones;

    /** PCG spawn rules per biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TArray<FWorld_PCGSpawnRule> SpawnRules;

    /**
     * Query which biome a world location falls into.
     * Returns EWorld_BiomeType::None if outside all zones.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /**
     * Get full biome config for a location.
     * Returns false if location is outside all biome zones.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool GetBiomeConfigAtLocation(FVector WorldLocation, FWorld_BiomeConfig& OutConfig) const;

    /**
     * Get all PCG spawn rules for a specific biome.
     */
    UFUNCTION(BlueprintCallable, Category = "PCG")
    TArray<FWorld_PCGSpawnRule> GetSpawnRulesForBiome(EWorld_BiomeType BiomeType) const;

    /**
     * Get ambient temperature at a world location (interpolated at biome edges).
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    /**
     * Get danger level at a world location.
     */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDangerLevelAtLocation(FVector WorldLocation) const;

    /**
     * Editor utility — populate default prehistoric biome configs.
     */
    UFUNCTION(CallInEditor, Category = "Biome")
    void SetupDefaultPrehistoricBiomes();

protected:
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    /** Find the nearest biome zone to a location (returns index or -1) */
    int32 FindBiomeIndexAtLocation(FVector WorldLocation) const;
};
