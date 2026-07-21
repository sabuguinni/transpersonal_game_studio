#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "PCGBiomeManager.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UHierarchicalInstancedStaticMeshComponent;

/** Biome type enum for the Cretaceous world */
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Cretaceous Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Rocky       UMETA(DisplayName = "Rocky Cliffs"),
    River       UMETA(DisplayName = "River Valley"),
    Volcanic    UMETA(DisplayName = "Volcanic Region")
};

/** Data for a single biome zone */
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 5000.0f;  // instances per km²

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float MinHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float MaxHeight = 1000.0f;
};

/**
 * PCGBiomeManager — manages biome zones and procedural placement rules
 * for the Cretaceous prehistoric world.
 * Agent #05 — Procedural World Generator
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APCGBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** All biome zones in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    /** World size in UE units (square) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float WorldSize = 8400.0f;

    /** Terrain grid resolution (cells per side) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    int32 TerrainGridSize = 7;

    /** Cell size in UE units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float CellSize = 1200.0f;

    /** Max foliage instances per km² (performance cap from Agent #04) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Performance")
    float MaxFoliagePerKm2 = 10000.0f;

    /** Max visible triangles budget */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Performance")
    int32 MaxVisibleTriangles = 500000;

    /** Get biome type at a world location */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Get foliage density at a world location */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetFoliageDensityAtLocation(FVector WorldLocation) const;

    /** Get terrain height at grid coordinates using pseudo-Perlin */
    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    float GetTerrainHeightAtGrid(int32 GridX, int32 GridY) const;

    /** Initialize default Cretaceous biome zones */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biomes")
    void InitializeDefaultBiomes();

    /** Get all biome zones of a specific type */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    TArray<FWorld_BiomeZone> GetBiomeZonesByType(EWorld_BiomeType Type) const;

private:
    /** Pseudo-Perlin noise helper */
    float PseudoPerlin(float X, float Y, float Seed = 42.0f) const;
};
