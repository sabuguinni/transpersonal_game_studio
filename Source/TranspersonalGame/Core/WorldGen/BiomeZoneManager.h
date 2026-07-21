// BiomeZoneManager.h
// Procedural biome zone management for Cretaceous world
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260623_005

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeZoneManager.generated.h"

// ── Biome type enum ───────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Plains      UMETA(DisplayName = "Open Plains"),
    Forest      UMETA(DisplayName = "Dense Forest"),
    RockyHills  UMETA(DisplayName = "Rocky Hills"),
    River       UMETA(DisplayName = "River Valley"),
    Volcanic    UMETA(DisplayName = "Volcanic Zone"),
    Swamp       UMETA(DisplayName = "Swamp"),
    COUNT       UMETA(Hidden)
};

// ── Biome zone data ───────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float MinElevation = -200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float MaxElevation = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinoSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");
};

// ── BiomeZoneManager Actor ────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, ClassGroup = "WorldGen")
class TRANSPERSONALGAME_API ABiomeZoneManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeZoneManager();

    // ── Biome zones ───────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    // ── Performance budget (from PerformanceBudget.h) ─────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Performance")
    float FoliageCullDistanceCM = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Performance")
    float WorldPartitionCellSizeCM = 25600.0f;  // 256m — matches streaming radius

    // ── Queries ───────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    FWorld_BiomeZone GetBiomeZoneAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetElevationAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    bool IsInRiverZone(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biomes")
    float GetDinoSpawnWeightAtLocation(FVector WorldLocation) const;

    // ── Initialisation ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biomes")
    void InitialiseDefaultBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biomes")
    void LogBiomeSummary() const;

protected:
    virtual void BeginPlay() override;

private:
    // Default biome layout matching MinPlayableMap
    void SetupCretaceousBiomes();
};
