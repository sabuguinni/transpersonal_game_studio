#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "PCGBiomeSystem.generated.h"

/**
 * Biome type enum for the prehistoric world.
 * Four distinct ecological zones placed in cardinal quadrants of the map.
 */
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Jurassic Forest"),
    Plains      UMETA(DisplayName = "Volcanic Plains"),
    Highlands   UMETA(DisplayName = "Rocky Highlands"),
    Wetlands    UMETA(DisplayName = "River Wetlands"),
    Undefined   UMETA(DisplayName = "Undefined")
};

/**
 * Data for a single biome zone.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Undefined;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float Radius = 2500.0f;

    /** Ambient temperature in Celsius — affects player survival stats */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float AmbientTemperature = 25.0f;

    /** Humidity 0-1 — affects thirst drain rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float Humidity = 0.5f;

    /** Danger level 0-1 — scales predator spawn frequency */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float DangerLevel = 0.3f;

    /** Foliage density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float FoliageDensity = 1.0f;
};

/**
 * PCGBiomeSystem — manages biome zone classification and queries.
 * Placed once in the level; other systems query it for biome data at any world position.
 * 
 * Biome layout (MinPlayableMap):
 *   Forest    (-2000, -2000) — NW — dense ferns, cycads, high humidity
 *   Plains    ( 2000, -2000) — NE — open grassland, volcanic soil, low cover
 *   Highlands ( 2000,  2000) — SE — rocky cliffs, boulders, cold nights
 *   Wetlands  (-2000,  2000) — SW — river delta, shallow water, reed beds
 */
UCLASS(ClassGroup = "World", meta = (DisplayName = "PCG Biome System"))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    /** All biome zones in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    TArray<FWorld_BiomeZone> BiomeZones;

    /**
     * Returns the biome type at the given world position.
     * Uses distance-to-center with radius check; returns Undefined if outside all zones.
     */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns the full biome zone data at the given world position.
     * Returns default FWorld_BiomeZone (Undefined) if outside all zones.
     */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    FWorld_BiomeZone GetBiomeZoneAtLocation(const FVector& WorldLocation) const;

    /**
     * Returns the nearest biome zone regardless of radius.
     * Useful for blending biome properties at borders.
     */
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetNearestBiome(const FVector& WorldLocation) const;

    /** Debug: draw biome zone boundaries in viewport */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biome")
    void DrawBiomeDebug();

protected:
    virtual void BeginPlay() override;

private:
    void InitializeDefaultZones();
};
