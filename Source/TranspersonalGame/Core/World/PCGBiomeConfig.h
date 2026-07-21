#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PCGBiomeConfig.generated.h"

/**
 * Biome types for the Cretaceous world.
 * Defined in PCGBiomeConfig.h — do NOT redefine elsewhere.
 */
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Cretaceous Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Rocky       UMETA(DisplayName = "Rocky Volcanic"),
    River       UMETA(DisplayName = "River Delta"),
    Swamp       UMETA(DisplayName = "Swamp / Wetland"),
    Count       UMETA(Hidden)
};

/**
 * Per-biome PCG density and visual parameters.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    /** Tree density: instances per 100m² */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation", meta = (ClampMin = "0", ClampMax = "50"))
    float TreeDensity = 10.0f;

    /** Bush/shrub density: instances per 100m² */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation", meta = (ClampMin = "0", ClampMax = "100"))
    float BushDensity = 20.0f;

    /** Rock outcrop density: instances per 100m² */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain", meta = (ClampMin = "0", ClampMax = "30"))
    float RockDensity = 5.0f;

    /** Average tree height scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Vegetation", meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float TreeHeightScale = 1.0f;

    /** Ground colour tint for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor GroundTint = FLinearColor(0.3f, 0.5f, 0.2f, 1.0f);

    /** Fog density multiplier for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Atmosphere", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float FogDensityMultiplier = 1.0f;

    /** Whether water is present in this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Water")
    bool bHasWater = false;

    /** Dinosaur spawn weight for this biome (relative probability) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Fauna", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DinoSpawnWeight = 0.5f;
};

/**
 * UPCGBiomeConfig — DataAsset holding all biome parameter sets.
 * Assign in Project Settings or BiomeManager.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPCGBiomeConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPCGBiomeConfig();

    /** Biome parameter table indexed by EWorld_BiomeType */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeParams> BiomeTable;

    /** Returns params for a given biome type. Falls back to Plains if not found. */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeParams GetBiomeParams(EWorld_BiomeType BiomeType) const;

    /** Returns the biome type at a world location using overlap with BiomeVolumes. */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;
};
