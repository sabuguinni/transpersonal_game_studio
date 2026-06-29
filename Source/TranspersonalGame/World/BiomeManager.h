#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// Biome zone data — defines a rectangular biome region
// ============================================================
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D CenterXY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D ExtentXY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;   // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterPresence;       // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurDensity;     // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;   // Debug visualization color

    FWorld_BiomeZone()
        : BiomeName(TEXT("Unknown"))
        , CenterXY(FVector2D::ZeroVector)
        , ExtentXY(FVector2D(1000.f, 1000.f))
        , BaseElevation(0.f)
        , VegetationDensity(0.5f)
        , WaterPresence(0.f)
        , DinosaurDensity(0.3f)
        , BiomeColor(FLinearColor::White)
    {}
};

// ============================================================
// Dinosaur spawn entry for a biome
// ============================================================
USTRUCT(BlueprintType)
struct FWorld_DinoSpawnEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 MinCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 MaxCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnWeight;  // Relative probability

    FWorld_DinoSpawnEntry()
        : SpeciesName(TEXT("Raptor"))
        , MinCount(1)
        , MaxCount(3)
        , SpawnWeight(1.0f)
    {}
};

// ============================================================
// ABiomeManager — manages biome zones, PCG scatter, dino spawn
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Biome Registry ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    bool bAutoRegisterDefaultBiomes;

    // ---- Vegetation PCG ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 VegetationSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float GlobalVegetationScale;

    // ---- Dinosaur Spawning ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    TArray<FWorld_DinoSpawnEntry> GlobalSpawnTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    int32 DinosaurSpawnSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    bool bSpawnDinosaursOnBeginPlay;

    // ---- Runtime State ----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 TotalActorsSpawned;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 ActiveBiomeCount;

    // ---- API ----

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterDefaultBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FString GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetVegetationDensityAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetWaterPresenceAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void ScatterVegetationInBiome(int32 BiomeIndex, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SpawnDinosaursInBiome(int32 BiomeIndex);

    UFUNCTION(BlueprintCallable, Category = "Biome", CallInEditor)
    void RegenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FVector> GeneratePoissonDiskPoints(FVector2D Center, FVector2D Extent, float MinDistance, int32 MaxPoints, int32 Seed) const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawBiomeDebugBounds(float Duration = 5.0f) const;

private:
    const FWorld_BiomeZone* FindBiomeAtLocation(FVector WorldLocation) const;
    FVector GetRandomPointInBiome(const FWorld_BiomeZone& Zone, int32& InOutSeed) const;
};
