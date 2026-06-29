#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// ============================================================
// Biome type enum — P1 World Generation
// ============================================================
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Jungle        UMETA(DisplayName = "Jungle"),
    Savanna       UMETA(DisplayName = "Savanna"),
    Swamp         UMETA(DisplayName = "Swamp"),
    Volcanic      UMETA(DisplayName = "Volcanic"),
    Coastal       UMETA(DisplayName = "Coastal"),
    Forest        UMETA(DisplayName = "Forest"),
    Plains        UMETA(DisplayName = "Plains"),
    Unknown       UMETA(DisplayName = "Unknown")
};

// ============================================================
// Per-biome data row for DataTable
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");
};

// ============================================================
// BiomeManager — world actor that classifies terrain into biomes
// and drives environment parameters (fog, temperature, vegetation)
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ---- Blueprint-callable API ----

    /** Returns the biome type at a given world location using height + noise sampling */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full biome data struct for the given biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType) const;

    /** Forces a biome refresh across the entire world (call after terrain changes) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void RefreshBiomeMap();

    /** Returns the dominant biome within a radius around a point */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetDominantBiomeInRadius(const FVector& Center, float Radius) const;

    // ---- Properties ----

    /** World size in cm — used for biome sampling grid */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldSizeCm = 400000.0f;

    /** Grid resolution for biome sampling (cells per axis) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    int32 BiomeGridResolution = 64;

    /** Noise scale for biome boundary variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeNoiseScale = 0.0001f;

    /** Optional DataTable override for biome parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    UDataTable* BiomeDataTable = nullptr;

    /** Height threshold below which terrain is classified as Coastal/Swamp */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Thresholds")
    float CoastalHeightThreshold = 500.0f;

    /** Height threshold above which terrain is classified as Volcanic */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Thresholds")
    float VolcanicHeightThreshold = 8000.0f;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    /** Internal biome grid — flattened 2D array of biome types */
    TArray<EEng_BiomeType> BiomeGrid;

    /** Populate default biome data for all biome types */
    void InitDefaultBiomeData();

    /** Sample biome at normalised UV coordinates [0,1] */
    EEng_BiomeType SampleBiomeAtUV(float U, float V) const;

    /** Simple deterministic noise for biome boundary variation */
    float SimpleNoise(float X, float Y) const;

    /** Default biome data map (used when no DataTable is set) */
    TMap<EEng_BiomeType, FEng_BiomeData> DefaultBiomeData;
};
