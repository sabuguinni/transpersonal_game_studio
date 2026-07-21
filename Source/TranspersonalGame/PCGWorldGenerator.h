// Copyright Transpersonal Game Studio. All Rights Reserved.
// PCGWorldGenerator.h — Agent #5 Procedural World Generator
// Biome layout, foliage spawning, water body placement, NavMesh config.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "PCGWorldGenerator.generated.h"

// ============================================================
// EWorld_BiomeType — biome classification enum
// Prefix: World_ (Agent #5 namespace)
// ============================================================
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    ForestValley    UMETA(DisplayName = "Forest Valley"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    Swampland       UMETA(DisplayName = "Swampland"),
    Coastline       UMETA(DisplayName = "Coastline")
};

// ============================================================
// FWorld_BiomeZone — data for a single biome region
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
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unnamed Biome");
};

// ============================================================
// APCGWorldGenerator — main PCG actor
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = (WorldGen))
class TRANSPERSONALGAME_API APCGWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    APCGWorldGenerator();

protected:
    virtual void BeginPlay() override;

public:
    // ---- World Generation Entry Points ----

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void RegenerateWorld();

    // ---- Biome System ----

    UFUNCTION(BlueprintCallable, Category = "World Generation|Biomes")
    void GenerateBiomeLayout();

    UFUNCTION(BlueprintCallable, Category = "World Generation|Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation|Biomes")
    float GetFoliageDensityAtLocation(const FVector& WorldLocation) const;

    // ---- Foliage ----

    UFUNCTION(BlueprintCallable, Category = "World Generation|Foliage")
    void SpawnFoliageForBiome(const FWorld_BiomeZone& BiomeZone);

    UFUNCTION(BlueprintCallable, Category = "World Generation|Foliage")
    void SpawnFoliageForAllBiomes();

    // ---- Water ----

    UFUNCTION(BlueprintCallable, Category = "World Generation|Water")
    void PlaceWaterBodies();

    // ---- Navigation ----

    UFUNCTION(BlueprintCallable, Category = "World Generation|Navigation")
    void ConfigureNavMesh();

    // ---- Terrain ----

    UFUNCTION(BlueprintCallable, Category = "World Generation|Terrain")
    void ApplyHeightVariation();

    // ---- Seed ----

    UFUNCTION(BlueprintCallable, Category = "World Generation|Seed")
    void SetWorldSeed(int32 NewSeed);

    // ---- Properties ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 WorldSeedValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    bool bGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float WorldScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float BiomeBlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float WaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Biomes")
    TMap<EWorld_BiomeType, float> BiomeWeights;
};
