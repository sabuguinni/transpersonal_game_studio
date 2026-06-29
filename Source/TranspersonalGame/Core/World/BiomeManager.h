#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// Biome type enum — unique prefix World_ per RULE 2
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Dense Forest"),
    RockyPlains UMETA(DisplayName = "Rocky Plains"),
    Savanna     UMETA(DisplayName = "Open Savanna"),
    RiverValley UMETA(DisplayName = "River Valley"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcano     UMETA(DisplayName = "Volcanic Region"),
    COUNT       UMETA(Hidden)
};

// Per-biome configuration data
USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString DisplayName = TEXT("Unknown Biome");

    // Temperature in Celsius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "-20.0", ClampMax = "60.0"))
    float AmbientTemperature = 25.0f;

    // Humidity 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.5f;

    // Fog density multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float FogDensityMultiplier = 1.0f;

    // Foliage density 0-1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Foliage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FoliageDensity = 0.7f;

    // Dinosaur spawn weight for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DinosaurSpawnWeight = 0.5f;

    // Water presence (rivers, lakes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Water")
    bool bHasWater = false;

    // Ambient sound tag for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    FName AmbientSoundTag = NAME_None;
};

/**
 * ABiomeManager — manages biome zones, transitions and per-biome properties.
 * Placed once in the level; queried by AI, weather, audio and foliage systems.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // All configured biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    // Radius (cm) used for biome-zone detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (ClampMin = "100.0"))
    float BiomeDetectionRadius = 50000.0f;

    // Returns the biome type at a given world location
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Returns the full config for a biome type
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeConfig GetBiomeConfig(EWorld_BiomeType BiomeType) const;

    // Returns temperature at a world location
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // Returns humidity at a world location
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    // Returns true if location is inside a water biome
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool IsWaterBiome(const FVector& WorldLocation) const;

    // Debug: draw biome zones in viewport
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biomes|Debug")
    void DrawBiomeDebugZones();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void InitDefaultBiomes();
};
