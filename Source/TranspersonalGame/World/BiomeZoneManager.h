#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeZoneManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky"),
    River       UMETA(DisplayName = "River"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight;

    FWorld_BiomeZone()
        : Center(FVector::ZeroVector)
        , Radius(2000.f)
        , BiomeType(EWorld_BiomeType::Plains)
        , BiomeName(TEXT("Default"))
        , FoliageDensity(0.5f)
        , DinosaurSpawnWeight(1.0f)
    {}
};

/**
 * ABiomeZoneManager
 * Manages distinct biome zones in the MinPlayableMap world.
 * Defines jungle, plains, rocky, and river zones with associated
 * foliage density, dinosaur spawn weights, and environmental properties.
 *
 * World layout (PROD_CYCLE_AUTO_20260625_006):
 *   JUNGLE_ZONE:  center=(2000,2000,0)   radius=3000u — 50 trees, T-Rex/Raptor spawn
 *   ROCKY_ZONE:   center=(5000,-3000,0)  radius=1500u — 20 rock formations
 *   PLAINS_ZONE:  center=(-2000,-2000,0) radius=2500u — open raptor navigation
 *   RIVER_ZONE:   center=(8000,0,0)      radius=800u  — tutorial water objective
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "World")
class TRANSPERSONALGAME_API ABiomeZoneManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Biome Zone Data ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    // --- Query API ---

    /** Returns the dominant biome type at the given world location */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Returns the biome zone data for the dominant biome at location */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeZone GetBiomeZoneAtLocation(FVector WorldLocation) const;

    /** Returns all biome zones of a given type */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<FWorld_BiomeZone> GetZonesOfType(EWorld_BiomeType BiomeType) const;

    /** Returns true if the location is within any biome zone */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool IsLocationInBiome(FVector WorldLocation, EWorld_BiomeType BiomeType) const;

    /** Returns the foliage density multiplier at a given location (0.0 - 1.0) */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetFoliageDensityAtLocation(FVector WorldLocation) const;

    /** Returns the dinosaur spawn weight at a given location */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetDinoSpawnWeightAtLocation(FVector WorldLocation) const;

    /** Registers a new biome zone at runtime */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void RegisterBiomeZone(FWorld_BiomeZone NewZone);

    // --- Debug ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawBiomeDebug;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DrawBiomeZones();

protected:
    void InitializeDefaultBiomes();

private:
    /** Find the closest biome zone to a location, returns index or -1 */
    int32 FindDominantBiomeIndex(FVector WorldLocation) const;
};
