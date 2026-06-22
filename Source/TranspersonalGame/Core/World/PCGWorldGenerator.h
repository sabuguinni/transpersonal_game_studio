// PCGWorldGenerator.h
// Transpersonal Game Studio — Prehistoric Survival Game
// Agent #05 — Procedural World Generator
// Biome classification, terrain height sampling, PCG spawn point generation.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "PCGWorldGenerator.generated.h"

// ============================================================
// EWorld_BiomeType — biome classification enum
// ============================================================
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    RiverValley   UMETA(DisplayName = "River Valley"),
    OpenPlains    UMETA(DisplayName = "Open Plains"),
    DenseJungle   UMETA(DisplayName = "Dense Jungle"),
    RockyUpland   UMETA(DisplayName = "Rocky Upland"),
    CliffFace     UMETA(DisplayName = "Cliff Face"),
    Wetlands      UMETA(DisplayName = "Wetlands"),
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
    float RadiusMeters = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationNormalized = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float MoistureNormalized = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;
};

// ============================================================
// APCGWorldGenerator — main world generation actor
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "PCG World Generator"))
class TRANSPERSONALGAME_API APCGWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    APCGWorldGenerator();

protected:
    virtual void BeginPlay() override;

public:
    // --- World Parameters ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 WorldSeedValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float WorldScaleKm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Generation")
    bool bBiomesGenerated;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Generation")
    bool bTerrainHeightCached;

    // --- Biome Elevation Thresholds ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Thresholds")
    float RiverValleyMaxElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Thresholds")
    float PlainsMaxElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Thresholds")
    float ForestMaxElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Thresholds")
    float RockyUplandMinElevation;

    // --- Runtime Biome Data ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Generation")
    TArray<FWorld_BiomeZone> BiomeZones;

    // --- Core Methods ---
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeMap();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetTerrainHeightAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<FTransform> GenerateSpawnPoints(EWorld_BiomeType BiomeType, int32 Count) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    int32 GetBiomeCount() const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    int32 GetWorldSeed() const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetWorldSeed(int32 NewSeed);

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "World Generation")
    void RegenerateBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<FWorld_BiomeZone> GetBiomeZones() const;
};
