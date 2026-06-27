// PCGBiomeSystem.h
// Prehistoric Dinosaur Survival Game — PCG Biome System
// Agent #5 — Procedural World Generator
// Defines biome zones, terrain variation, and procedural placement rules

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGBiomeSystem.generated.h"

// Biome types for the prehistoric world
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    DenseJungle     UMETA(DisplayName = "Dense Jungle"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    Swampland       UMETA(DisplayName = "Swampland"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
};

// Biome configuration data
USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterPresence = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainRoughness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FName> AllowedDinosaurSpecies;
};

// PCG placement point for procedural asset spawning
USTRUCT(BlueprintType)
struct FWorld_PCGPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "PCG")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "PCG")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "PCG")
    FVector Scale = FVector::OneVector;

    UPROPERTY(BlueprintReadWrite, Category = "PCG")
    EWorld_BiomeType Biome = EWorld_BiomeType::OpenPlains;

    UPROPERTY(BlueprintReadWrite, Category = "PCG")
    float Density = 1.0f;
};

/**
 * UPCGBiomeSystem — Manages procedural biome zones and placement rules
 * Defines the geographic structure of the prehistoric world.
 * Biomes determine vegetation, dinosaur species, terrain, and weather.
 */
UCLASS(ClassGroup = "World", meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UPCGBiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UPCGBiomeSystem();

    // Biome configuration array — define all biomes in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    // Maximum number of PCG points to generate per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Generation")
    int32 MaxPointsPerBiome = 200;

    // Random seed for deterministic generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Generation")
    int32 WorldSeed = 42;

    // Minimum distance between placed objects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Generation")
    float MinPlacementDistance = 150.0f;

    // Whether to use Poisson disk sampling for placement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Generation")
    bool bUsePoissonDiskSampling = true;

    // Get biome type at a world location
    UFUNCTION(BlueprintCallable, Category = "PCG|Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    // Get biome config for a given type
    UFUNCTION(BlueprintCallable, Category = "PCG|Biomes")
    FWorld_BiomeConfig GetBiomeConfig(EWorld_BiomeType BiomeType) const;

    // Generate PCG placement points for a biome
    UFUNCTION(BlueprintCallable, Category = "PCG|Generation")
    TArray<FWorld_PCGPoint> GenerateBiomePoints(const FWorld_BiomeConfig& Config) const;

    // Generate all biome points for the entire world
    UFUNCTION(BlueprintCallable, Category = "PCG|Generation")
    TArray<FWorld_PCGPoint> GenerateAllWorldPoints() const;

    // Check if a location is valid for placement (no overlap)
    UFUNCTION(BlueprintCallable, Category = "PCG|Generation")
    bool IsValidPlacementLocation(FVector Location, const TArray<FWorld_PCGPoint>& ExistingPoints) const;

    // Get vegetation density multiplier for a biome
    UFUNCTION(BlueprintPure, Category = "PCG|Biomes")
    float GetVegetationDensity(EWorld_BiomeType BiomeType) const;

    // Get water presence factor for a biome (0-1)
    UFUNCTION(BlueprintPure, Category = "PCG|Biomes")
    float GetWaterPresence(EWorld_BiomeType BiomeType) const;

    // Initialize default biome configurations for the prehistoric world
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "PCG|Setup")
    void InitializeDefaultBiomes();

    // Debug: draw biome boundaries in editor
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "PCG|Debug")
    void DrawBiomeBoundaries();

protected:
    virtual void BeginPlay() override;

private:
    // Internal Poisson disk sampling
    TArray<FVector2D> GeneratePoissonDiskSamples(FVector2D AreaMin, FVector2D AreaMax, float MinDist, int32 MaxSamples) const;

    // Cached biome lookup for performance
    mutable TMap<EWorld_BiomeType, int32> BiomeIndexCache;
    void RebuildBiomeCache() const;
};
