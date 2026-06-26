// PCGBiomeSystem.h — Transpersonal Game Studio
// Agent #05 — Procedural World Generator — Cycle PROD_CYCLE_AUTO_20260626_003
// Biome system: defines biome types, terrain features, and foliage distribution rules
// Uses World_ prefix on all types to avoid cross-agent collisions (Rule 2)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PCGBiomeSystem.generated.h"

// ─── Biome Type Enum ───────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    TropicalForest  UMETA(DisplayName = "Tropical Forest"),
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    Wetlands        UMETA(DisplayName = "Wetlands"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    VolcanicPlains  UMETA(DisplayName = "Volcanic Plains"),
};

// ─── Water Body Type ──────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_WaterBodyType : uint8
{
    None    UMETA(DisplayName = "None"),
    River   UMETA(DisplayName = "River"),
    Lake    UMETA(DisplayName = "Lake"),
    Swamp   UMETA(DisplayName = "Swamp"),
    Ocean   UMETA(DisplayName = "Ocean"),
};

// ─── Terrain Feature Struct ───────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    EWorld_BiomeType Biome = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float HeightOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    bool bIsWaterBody = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    EWorld_WaterBodyType WaterType = EWorld_WaterBodyType::None;
};

// ─── Biome Config Struct ──────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FLinearColor FogColor = FLinearColor(0.5f, 0.7f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float TreeDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float RockDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float WaterPresence = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float MinTerrainHeight = -100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float MaxTerrainHeight = 500.0f;

    // Foliage cull distance — must align with PerformanceConfig.h PERF_FOLIAGE_CULL_DISTANCE (8000.0f)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float FoliageCullDistance = 8000.0f;
};

// ─── PCG Biome System Actor ───────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "PCG Biome System"))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    // ─── Biome Configuration ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType ActiveBiome = EWorld_BiomeType::TropicalForest;

    // ─── Terrain Features ─────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    int32 RockFormationCount = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    int32 WaterBodyCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    int32 TreeClusterCount = 30;

    // ─── World Bounds ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Bounds")
    FVector WorldCenter = FVector(2000.0f, 2000.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Bounds")
    float WorldRadius = 4000.0f;

    // ─── Performance ──────────────────────────────────────────────────────
    // Aligned with PerformanceConfig.h PERF_FOLIAGE_CULL_DISTANCE
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Performance")
    float FoliageCullDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Performance")
    float RockCullDistance = 10000.0f;

    // ─── Functions ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    FWorld_BiomeConfig GetBiomeConfig(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Terrain")
    TArray<FWorld_TerrainFeature> GetTerrainFeaturesInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Generation")
    void GenerateBiomeDefaults();

    UFUNCTION(BlueprintCallable, Category = "World|Water")
    bool IsLocationInWater(FVector WorldLocation) const;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    void InitializeDefaultBiomes();
};
