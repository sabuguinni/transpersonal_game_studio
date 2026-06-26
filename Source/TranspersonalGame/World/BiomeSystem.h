// BiomeSystem.h — Agent #5 Procedural World Generator
// Defines biome zones, terrain features, and water bodies for the prehistoric world
// Cycle: PROD_CYCLE_AUTO_20260626_009

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BiomeSystem.generated.h"

// ============================================================
// ENUMS — Global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    DenseTropicalForest     UMETA(DisplayName = "Dense Tropical Forest"),
    OpenPlains              UMETA(DisplayName = "Open Plains"),
    RockyHighlands          UMETA(DisplayName = "Rocky Highlands"),
    RiverValley             UMETA(DisplayName = "River Valley"),
    LakeShore               UMETA(DisplayName = "Lake Shore"),
    VolcanicZone            UMETA(DisplayName = "Volcanic Zone"),
    Swampland               UMETA(DisplayName = "Swampland"),
    COUNT                   UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWorld_TerrainFeature : uint8
{
    FlatGround      UMETA(DisplayName = "Flat Ground"),
    GentleHill      UMETA(DisplayName = "Gentle Hill"),
    SteepCliff      UMETA(DisplayName = "Steep Cliff"),
    RiverBed        UMETA(DisplayName = "River Bed"),
    LakeBasin       UMETA(DisplayName = "Lake Basin"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    COUNT           UMETA(Hidden)
};

// ============================================================
// STRUCTS — Global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationVariance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor DebugColor = FLinearColor::Green;
};

USTRUCT(BlueprintType)
struct FWorld_WaterBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Extent = FVector(1000.0f, 1000.0f, 10.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bIsRiver = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float FlowSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FLinearColor WaterColor = FLinearColor(0.1f, 0.3f, 0.8f, 0.85f);
};

USTRUCT(BlueprintType)
struct FWorld_TerrainConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseFrequency = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 OctaveCount = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Persistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Lacunarity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 RandomSeed = 42;
};

// ============================================================
// UCLASS: ABiomeManager
// ============================================================

UCLASS(ClassGroup = (World), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ---- Biome Zones ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    // ---- Water Bodies ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    TArray<FWorld_WaterBody> WaterBodies;

    // ---- Terrain Config ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FWorld_TerrainConfig TerrainConfig;

    // ---- World Bounds ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float WorldRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 BiomeCount = 5;

    // ---- Runtime State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime",
              meta = (AllowPrivateAccess = "true"))
    bool bBiomesInitialized = false;

    // ---- Functions ----
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetVegetationDensityAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Water")
    bool IsLocationInWater(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetTerrainHeightAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawBiomeBoundaries();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    void SetupDefaultBiomeZones();
    void SetupDefaultWaterBodies();
    float PerlinNoise2D(float X, float Y, int32 Seed) const;
};
