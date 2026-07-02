#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BiomeManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Biome type enum — all prehistoric biomes in the game world
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Jungle Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Swamp       UMETA(DisplayName = "Swampland"),
    Volcanic    UMETA(DisplayName = "Volcanic Zone"),
    River       UMETA(DisplayName = "River Corridor"),
    None        UMETA(DisplayName = "Undefined")
};

// ─────────────────────────────────────────────────────────────────────────────
// Biome definition data — per-biome parameters for PCG and environment
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseElevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationVariance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;      // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;          // 0.0 dry → 1.0 saturated

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f; // 0.0 barren → 1.0 dense

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor DebugColor = FLinearColor::White;
};

// ─────────────────────────────────────────────────────────────────────────────
// Biome query result — returned when sampling world position
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeQueryResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType PrimaryBiome = EWorld_BiomeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType SecondaryBiome = EWorld_BiomeType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendFactor = 0.0f;   // 0 = pure primary, 1 = pure secondary

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float VegetationDensity = 0.5f;
};

// ─────────────────────────────────────────────────────────────────────────────
// ABiomeManager — world actor that manages all biome zones
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ── Registered biome zones ────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeData> BiomeZones;

    // ── Blend radius between biomes ───────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes", meta = (ClampMin = "100.0"))
    float BiomeBlendRadius = 500.0f;

    // ── Debug visualization ───────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Debug")
    bool bDrawBiomeDebug = false;

    // ── Query API ─────────────────────────────────────────────────────

    /** Returns the biome data at a given world position */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FWorld_BiomeQueryResult QueryBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns the dominant biome type at a position */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeType GetBiomeTypeAtLocation(const FVector& WorldLocation) const;

    /** Returns temperature at world position (blended between biomes) */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Returns humidity at world position */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    /** Returns vegetation density at world position */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    /** Registers a new biome zone at runtime */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    void RegisterBiomeZone(const FWorld_BiomeData& BiomeData);

    /** Initializes default prehistoric biome layout */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biomes")
    void InitializeDefaultBiomes();

    /** Returns all biome zones */
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    TArray<FWorld_BiomeData> GetAllBiomeZones() const { return BiomeZones; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    /** Finds the nearest biome zone to a world position */
    int32 FindNearestBiomeIndex(const FVector& WorldLocation) const;

    /** Finds the two nearest biomes for blending */
    void FindTwoNearestBiomes(const FVector& WorldLocation, int32& OutPrimary, int32& OutSecondary, float& OutBlend) const;

    /** Blends a float value between two biome zones */
    float BlendBiomeFloat(float ValueA, float ValueB, float BlendFactor) const;
};
