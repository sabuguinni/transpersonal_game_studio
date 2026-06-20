// BiomeManager.h
// Engine Architect #02 — P1 World Generation
// UGameInstanceSubsystem that classifies world locations into biome types
// and exposes terrain parameters (foliage density, danger level, ground colour)
// to all other systems via UFUNCTION calls.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Biome type enum  (Eng_ prefix — unique across project)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    CoastalWetlands     UMETA(DisplayName = "Coastal Wetlands"),
    DenseJungle         UMETA(DisplayName = "Dense Jungle"),
    OpenSavanna         UMETA(DisplayName = "Open Savanna"),
    VolcanicHighlands   UMETA(DisplayName = "Volcanic Highlands"),
    RiverDelta          UMETA(DisplayName = "River Delta"),
    AridBadlands        UMETA(DisplayName = "Arid Badlands"),
    TemperateForest     UMETA(DisplayName = "Temperate Forest"),
    MountainPeaks       UMETA(DisplayName = "Mountain Peaks"),
};

// ─────────────────────────────────────────────────────────────────────────────
// Per-biome data record
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::TemperateForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FText DisplayName;

    // Climate range this biome occupies
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float MinTemperature = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float MaxTemperature = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float MinHumidity = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float MaxHumidity = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float MinAltitude = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain")
    float MaxAltitude = 1000.f;

    // Gameplay parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterCoverage = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay", meta = (ClampMin = "1", ClampMax = "10"))
    int32 DangerLevel = 3;

    // Visual hint for terrain material blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor GroundColor = FLinearColor(0.3f, 0.25f, 0.1f, 1.f);
};

// ─────────────────────────────────────────────────────────────────────────────
// BiomeManager subsystem
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBiomeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // UGameInstanceSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Core queries (callable from Blueprint and C++) ──────────────────────

    /** Returns the biome type at a given world location. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Fills OutDefinition with the full data record for the given biome type. Returns false if not found. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool GetBiomeDefinition(EEng_BiomeType BiomeType, FEng_BiomeDefinition& OutDefinition) const;

    /** Returns foliage density [0..1] at the given world location. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetFoliageDensityAt(const FVector& WorldLocation) const;

    /** Returns danger level [1..10] at the given world location. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetDangerLevelAt(const FVector& WorldLocation) const;

    /** Returns the ground colour hint for terrain material blending. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FLinearColor GetGroundColorAt(const FVector& WorldLocation) const;

    /** Returns all registered biome types. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<EEng_BiomeType> GetAllBiomeTypes() const;

    /** Clears the location cache (call after major world changes). */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void InvalidateCache();

    // ── Data table (editable in editor) ─────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Data")
    TArray<FEng_BiomeDefinition> BiomeTable;

private:
    // Climate sampling helpers
    float SampleTemperature(const FVector& WorldLocation) const;
    float SampleHumidity(const FVector& WorldLocation) const;

    // Classification
    EEng_BiomeType ClassifyBiome(float Temperature, float Humidity, float Altitude) const;

    // Cache helpers
    FIntPoint WorldLocationToCacheKey(const FVector& WorldLocation) const;

    // Mutable cache (const queries update it)
    mutable TMap<FIntPoint, EEng_BiomeType> BiomeCache;

    bool bIsInitialized = false;
};
