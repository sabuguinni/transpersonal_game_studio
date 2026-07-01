// BiomeManager.h — Transpersonal Game Studio
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260701_003
// Biome classification system for Cretaceous survival world

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BiomeManager.generated.h"

// ============================================================
// UENUM — Biome types (must be at global scope, before UCLASS)
// Prefix: EEng_ to avoid collision with other agents' types
// ============================================================
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    CretaceousForest    UMETA(DisplayName = "Cretaceous Forest"),
    OpenPlains          UMETA(DisplayName = "Open Plains"),
    SwampDelta          UMETA(DisplayName = "Swamp Delta"),
    VolcanicBadlands    UMETA(DisplayName = "Volcanic Badlands"),
    CoastalShallows     UMETA(DisplayName = "Coastal Shallows"),
};

// ============================================================
// USTRUCT — Biome runtime data (global scope, before UCLASS)
// ============================================================
USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::CretaceousForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WeatherIntensity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor DebugColor = FLinearColor(0.13f, 0.55f, 0.13f, 1.0f);
};

// ============================================================
// ABiomeManager — World subsystem actor for biome queries
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldSizeKm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config",
        meta = (ClampMin = "8", ClampMax = "128"))
    int32 BiomeGridResolution;

    // --- Biome weights (sum should = 1.0) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TMap<EEng_BiomeType, float> BiomeWeights;

    // --- Runtime state ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Runtime")
    bool bBiomesInitialized;

    // --- Core API ---
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void InitializeBiomeGrid();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtWorldLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FString GetBiomeDisplayName(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetFoliageDensityForBiome(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDinosaurDensityForBiome(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetWeatherIntensityForBiome(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DebugDrawBiomeGrid();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // Flat array: BiomeGridResolution x BiomeGridResolution
    TArray<EEng_BiomeType> BiomeGrid;

    EEng_BiomeType ClassifyBiomeByPosition(float NormX, float NormY, float DistFromCenter, int32 Seed) const;
};
