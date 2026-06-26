// BiomeManager.h — Core Systems Programmer #03 — Cycle 008
// Prehistoric survival game: spatial biome registry.
// Manages biome definitions, species assignment, and location queries.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/SharedTypes.h"
#include "BiomeManager.generated.h"

/** Biome type classification */
UENUM(BlueprintType)
enum class ECore_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Canyon      UMETA(DisplayName = "Canyon"),
    Beach       UMETA(DisplayName = "Beach"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Unknown     UMETA(DisplayName = "Unknown"),
};

/** Data for a single biome region */
USTRUCT(BlueprintType)
struct FCore_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FName BiomeName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    ECore_BiomeType BiomeType = ECore_BiomeType::Unknown;

    /** World-space center of this biome region */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    /** Radius in world units (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.f;

    /** Danger level 0-1: drives dinosaur aggression scaling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.5f;

    /** Average temperature (Celsius) — affects survival mechanics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 28.f;

    /** Dinosaur species that spawn in this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FName> DinosaurSpecies;

    /** Foliage density multiplier for procedural world gen */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;
};

/**
 * ABiomeManager — Spatial registry of all biome regions in the world.
 * Non-ticking actor (biome data is static). Provides location queries used by:
 *   - PCGWorldGenerator (#05) for foliage density and terrain variation
 *   - DinosaurBase AI for aggression scaling via DangerLevel
 *   - SurvivalComponent for temperature-based survival penalties
 *
 * Usage: Place one ABiomeManager in the level. Call GetBiomeAtLocation() from any system.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;

    // ── Query API ──────────────────────────────────────────────────────────────

    /** Returns the biome data for the closest biome at the given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FCore_BiomeData GetBiomeAtLocation(FVector WorldLocation) const;

    /** Returns the biome type at the given world location */
    UFUNCTION(BlueprintPure, Category = "Biome|Query")
    ECore_BiomeType GetBiomeTypeAtLocation(FVector WorldLocation) const;

    /** Returns true if the location falls within the given biome's radius */
    UFUNCTION(BlueprintPure, Category = "Biome|Query")
    bool IsLocationInBiome(FVector WorldLocation, FName BiomeName) const;

    /** Returns the danger level (0-1) at the given world location */
    UFUNCTION(BlueprintPure, Category = "Biome|Query")
    float GetDangerLevelAtLocation(FVector WorldLocation) const;

    /** Returns the temperature at the given world location */
    UFUNCTION(BlueprintPure, Category = "Biome|Query")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    /** Returns all dinosaur species names that spawn in the biome at this location */
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    TArray<FName> GetDinosaursForBiome(FVector WorldLocation) const;

    /** Returns all registered biomes */
    UFUNCTION(BlueprintPure, Category = "Biome|Query")
    TArray<FCore_BiomeData> GetAllBiomes() const { return Biomes; }

    // ── Registration ──────────────────────────────────────────────────────────

    /** Add a biome at runtime (used by PCGWorldGenerator) */
    UFUNCTION(BlueprintCallable, Category = "Biome|Registration")
    void RegisterBiome(const FCore_BiomeData& BiomeData);

    /** Clear all biomes and re-register defaults */
    UFUNCTION(BlueprintCallable, Category = "Biome|Registration")
    void ResetToDefaults();

    // ── Editor utility ────────────────────────────────────────────────────────

    /** Draw debug spheres for all biome regions in the viewport */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DrawBiomeDebug();

    // ── Data ──────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Data")
    TArray<FCore_BiomeData> Biomes;

private:
    /** Populate default 4-biome layout for MinPlayableMap */
    void InitializeDefaultBiomes();

    /** Find the index of the closest biome to a world location. Returns -1 if none. */
    int32 FindClosestBiomeIndex(FVector WorldLocation) const;
};
