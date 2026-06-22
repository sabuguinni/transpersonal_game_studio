// PCGBiomeSystem.h
// Agent #05 — Procedural World Generator
// Biome classification and terrain generation system for Cretaceous world

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "PCGBiomeSystem.generated.h"

// ── Biome Types ──────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    JungleForest    UMETA(DisplayName = "Jungle Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    RockyBadlands   UMETA(DisplayName = "Rocky Badlands"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
    CoastalShore    UMETA(DisplayName = "Coastal Shore"),
    FernMeadow      UMETA(DisplayName = "Fern Meadow")
};

// ── Biome Cell Data ──────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeCell
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FVector2D GridCoord = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float TerrainHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float WaterLevel = -1000.0f;  // -1000 = no water

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    bool bHasRiver = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    bool bIsGenerated = false;
};

// ── Terrain Generation Config ────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_TerrainConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    int32 GridRows = 9;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    int32 GridColumns = 9;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float CellSize = 2000.0f;  // Unreal Units (20m)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float MaxHeightVariation = 1200.0f;  // UU

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float RiverDepth = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    float RiverWidth = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    bool bGenerateRiver = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    bool bGenerateRockyBadlands = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Terrain")
    FVector WorldOrigin = FVector(-8000.0f, -8000.0f, 0.0f);
};

// ── PCG Biome System Actor ───────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "PCG Biome System"))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    // ── Config ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    FWorld_TerrainConfig TerrainConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    TArray<FWorld_BiomeCell> BiomeCells;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    bool bAutoGenerateOnBeginPlay = false;

    // ── Generation ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void GenerateTerrain();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void ClearGeneratedTerrain();

    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    float GetTerrainHeightAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    bool IsLocationInRiver(FVector WorldLocation) const;

    // ── Stats ────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    int32 GetGeneratedCellCount() const;

    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    float GetHeightVariationRange() const;

protected:
    virtual void BeginPlay() override;

private:
    // Height calculation using multi-octave sine waves
    float CalculateTerrainHeight(float NormalizedX, float NormalizedY) const;

    // Biome classification from height + position
    EWorld_BiomeType ClassifyBiome(float NormalizedX, float NormalizedY, float Height) const;

    // Spawned terrain actors (tracked for cleanup)
    UPROPERTY()
    TArray<AActor*> SpawnedTerrainActors;

    float CachedMinHeight = 0.0f;
    float CachedMaxHeight = 0.0f;
};
