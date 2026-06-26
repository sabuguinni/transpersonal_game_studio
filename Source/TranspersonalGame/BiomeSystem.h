// BiomeSystem.h — Biome classification and environmental data for procedural world
// Agent #5 — Procedural World Generator — Cycle 013
// Defines biome types, properties, and the manager that assigns biomes to world regions.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

// ── Biome type enum ──────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    Swampland       UMETA(DisplayName = "Swampland"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
    CoastalShore    UMETA(DisplayName = "Coastal Shore"),
};

// ── Biome data struct ────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    // World-space center of this biome zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Location")
    FVector Center = FVector::ZeroVector;

    // Radius in cm of the biome influence zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Location")
    float Radius = 2000.0f;

    // Vegetation density 0-1 (drives PCG foliage spawning)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity = 0.5f;

    // Water presence 0-1 (rivers, lakes, swamps)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterPresence = 0.0f;

    // Elevation variance in cm (how hilly the terrain is)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float ElevationVariance = 200.0f;

    // Average temperature in Celsius (affects survival mechanics)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float AverageTemperatureCelsius = 22.0f;

    // Humidity 0-1 (affects thirst drain rate)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.5f;

    // Dinosaur spawn weight for this biome (relative probability)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Fauna")
    float DinosaurSpawnWeight = 1.0f;

    // Ambient sound tag for this biome (used by Audio Agent #16)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Audio")
    FName AmbientSoundTag = NAME_None;

    // Fog density override (0 = use global, >0 = biome-specific)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensityOverride = 0.0f;

    FWorld_BiomeData() {}
};

// ── Biome transition blend struct ────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeBlend
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType PrimaryBiome = EWorld_BiomeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType SecondaryBiome = EWorld_BiomeType::None;

    // 0 = fully primary, 1 = fully secondary
    UPROPERTY(BlueprintReadOnly, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendAlpha = 0.0f;

    FWorld_BiomeBlend() {}
};

// ── ABiomeManager actor ──────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ── Registered biome zones ────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeZones;

    // ── Transition blend distance (cm) ────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes|Settings")
    float TransitionBlendDistance = 500.0f;

    // ── Debug visualization ───────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes|Debug")
    bool bDrawBiomeBoundaries = false;

    // ── Query: get biome at world position ───────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    // ── Query: get blended biome data at world position ───────────────────
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeBlend GetBiomeBlendAtLocation(FVector WorldLocation) const;

    // ── Query: get full biome data for a type ─────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool GetBiomeData(EWorld_BiomeType BiomeType, FWorld_BiomeData& OutData) const;

    // ── Query: get vegetation density at location ─────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetVegetationDensityAtLocation(FVector WorldLocation) const;

    // ── Query: get temperature at location ───────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    // ── Register a new biome zone at runtime ─────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void RegisterBiomeZone(const FWorld_BiomeData& BiomeData);

    // ── Initialize default biomes for MinPlayableMap ─────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biomes")
    void InitializeDefaultBiomes();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    // Internal: find closest biome zone to a location
    int32 FindClosestBiomeIndex(const FVector& Location) const;

    // Internal: find two closest biomes for blending
    void FindTwoClosestBiomes(const FVector& Location, int32& OutPrimaryIdx, int32& OutSecondaryIdx, float& OutBlendAlpha) const;
};
