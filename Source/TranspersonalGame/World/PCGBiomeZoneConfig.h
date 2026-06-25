#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "PCGBiomeZoneConfig.generated.h"

/**
 * Defines the three primary biome zones for the Cretaceous world:
 *   Zone 0 — Dense Jungle  (center, around dinos, high vegetation density)
 *   Zone 1 — Open Savanna  (east, sparse trees, wide open plains)
 *   Zone 2 — Rocky Highlands (north, elevated terrain, boulder clusters)
 */

UENUM(BlueprintType)
enum class EWorld_BiomeZone : uint8
{
    Jungle    UMETA(DisplayName = "Dense Jungle"),
    Savanna   UMETA(DisplayName = "Open Savanna"),
    Highlands UMETA(DisplayName = "Rocky Highlands"),
    Riverbank UMETA(DisplayName = "River Bank"),
    MAX       UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeZone ZoneType = EWorld_BiomeZone::Jungle;

    /** Center of this biome zone in world space */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector ZoneCenter = FVector::ZeroVector;

    /** Radius of influence for this biome zone (Unreal units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ZoneRadius = 3000.0f;

    /** Vegetation density 0-1 (1 = maximum density) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity = 0.8f;

    /** Average terrain height variation in this zone (Unreal units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainHeightVariation = 200.0f;

    /** Rock/boulder density 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RockDensity = 0.3f;

    /** Fog density multiplier for this zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensityMultiplier = 1.0f;

    /** Primary foliage color tint */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FoliageTint = FLinearColor(0.2f, 0.6f, 0.1f, 1.0f);

    /** Whether this zone has water features (rivers/ponds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWaterFeatures = false;

    /** Dinosaur species that prefer this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FName> PreferredDinoSpecies;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPCGBiomeZoneConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPCGBiomeZoneConfig();

    /** All biome zones in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeZoneData> BiomeZones;

    /** Global world scale (1 unit = 1 cm in UE5) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Scale")
    float WorldScale = 1.0f;

    /** Total world bounds (square, centered at origin) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Scale")
    float WorldBoundsRadius = 10000.0f;

    /** Get the dominant biome zone at a given world location */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeZone GetBiomeAtLocation(FVector WorldLocation) const;

    /** Get the biome data for a specific zone type */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    bool GetBiomeData(EWorld_BiomeZone ZoneType, FWorld_BiomeZoneData& OutData) const;

    /** Initialize default Cretaceous world biome configuration */
    UFUNCTION(CallInEditor, Category = "World|Biomes")
    void InitializeDefaultCretaceousConfig();
};
