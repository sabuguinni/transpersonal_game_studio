#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "PCGBiomeSystem.generated.h"

// ─── Biome Type Enum ──────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    RiverValley     UMETA(DisplayName = "River Valley"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyUplands    UMETA(DisplayName = "Rocky Uplands"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Volcanic        UMETA(DisplayName = "Volcanic")
};

// ─── Biome Zone Data ──────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    bool bHasWaterSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FLinearColor DebugColor = FLinearColor::Green;
};

// ─── Scatter Point ────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_ScatterPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    FVector Scale = FVector::OneVector;

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    EWorld_BiomeType OwningBiome = EWorld_BiomeType::OpenPlains;
};

// ─── PCG Biome System Actor ───────────────────────────────────────────────────
/**
 * APCGBiomeSystem
 * Manages biome zone definitions and provides scatter point generation
 * for vegetation, rocks, and dinosaur spawn locations.
 * Agent #05 — Procedural World Generator
 */
UCLASS(ClassGroup = "World", meta = (DisplayName = "PCG Biome System"))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    // ── Biome Zones ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    float TransitionBlendRadius = 1000.0f;

    // ── PCG Scatter Settings ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    int32 VegetationScatterCount = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    int32 RockScatterCount = 80;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    int32 DinoSpawnPointCount = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    float MinScatterSpacing = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|PCG")
    int32 RandomSeed = 42;

    // ── Water Sources ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Water")
    TArray<FVector> WaterSourceLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Water")
    float RiverWidth = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Water")
    float RiverLength = 8000.0f;

    // ── Runtime State ─────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    TArray<FWorld_ScatterPoint> GeneratedVegetationPoints;

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    TArray<FWorld_ScatterPoint> GeneratedRockPoints;

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    TArray<FWorld_ScatterPoint> GeneratedDinoSpawnPoints;

    UPROPERTY(BlueprintReadOnly, Category = "World|PCG")
    bool bBiomesGenerated = false;

    // ── Public API ────────────────────────────────────────────────────────────

    /** Generate all scatter points for all biome zones */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void GenerateAllBiomes();

    /** Get biome type at a world location (returns nearest biome) */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Get blend weight for a specific biome at a location (0-1) */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    float GetBiomeBlendWeight(FVector WorldLocation, EWorld_BiomeType BiomeType) const;

    /** Get all dinosaur spawn points for a specific biome */
    UFUNCTION(BlueprintCallable, Category = "World|PCG")
    TArray<FWorld_ScatterPoint> GetDinoSpawnPointsForBiome(EWorld_BiomeType BiomeType) const;

    /** Get nearest water source to a location */
    UFUNCTION(BlueprintCallable, Category = "World|Water")
    FVector GetNearestWaterSource(FVector FromLocation) const;

    /** Check if location is within a water body */
    UFUNCTION(BlueprintCallable, Category = "World|Water")
    bool IsLocationInWater(FVector WorldLocation) const;

    /** Clear all generated scatter points */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void ClearGeneratedPoints();

    /** Register default biome zones (River Valley, Forest, Plains, Uplands) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|PCG")
    void RegisterDefaultBiomes();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    /** Generate scatter points within a single biome zone */
    void GenerateBiomeScatterPoints(const FWorld_BiomeZone& Zone);

    /** Check minimum spacing against existing points */
    bool IsSpacingValid(const FVector& NewPoint, const TArray<FWorld_ScatterPoint>& ExistingPoints) const;

    /** Pseudo-random float in range using seed */
    float SeededRandom(int32 Index, float Min, float Max) const;

    /** Pseudo-random angle using seed */
    float SeededAngle(int32 Index) const;
};
