#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BiomeManager.generated.h"

// Biome types for the prehistoric world
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Jungle         UMETA(DisplayName = "Jungle"),
    Savanna        UMETA(DisplayName = "Savanna"),
    Swamp          UMETA(DisplayName = "Swamp"),
    Volcanic       UMETA(DisplayName = "Volcanic"),
    Coastal        UMETA(DisplayName = "Coastal"),
    Forest         UMETA(DisplayName = "Forest"),
    Desert         UMETA(DisplayName = "Desert"),
    Tundra         UMETA(DisplayName = "Tundra"),
};

// Climate data per biome
USTRUCT(BlueprintType)
struct FEng_BiomeClimate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float TemperatureMin = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float TemperatureMax = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float HumidityPercent = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float RainfallMM = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float WindSpeedKMH = 15.0f;
};

// Biome definition — full data for a single biome region
USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Jungle");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FEng_BiomeClimate Climate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.8f;  // 0-1 scale

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.5f;  // 0-1 scale

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;
};

// Biome sample result at a world position
USTRUCT(BlueprintType)
struct FEng_BiomeSample
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType PrimaryBiome = EEng_BiomeType::Jungle;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType SecondaryBiome = EEng_BiomeType::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float BlendFactor = 0.0f;  // 0 = pure primary, 1 = pure secondary

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Humidity = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Altitude = 0.0f;
};

/**
 * ABiomeManager — manages biome regions, climate data, and biome sampling for the prehistoric world.
 * Placed once in the level. Other systems query it for biome data at world positions.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|World")
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // --- Biome Definitions ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EEng_BiomeType, FEng_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float WorldSizeKM = 10.0f;  // Total world size in km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeBlendRadius = 500.0f;  // UU blend radius at biome borders

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    int32 BiomeSeed = 12345;

    // --- Runtime State ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    int32 TotalBiomeRegions = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    bool bBiomesInitialized = false;

    // --- Core API ---

    /** Sample biome data at a world position */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FEng_BiomeSample SampleBiomeAtLocation(FVector WorldLocation) const;

    /** Get the primary biome type at a world position */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EEng_BiomeType GetBiomeTypeAtLocation(FVector WorldLocation) const;

    /** Get full biome definition for a biome type */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType BiomeType) const;

    /** Get temperature at a world position (accounts for altitude) */
    UFUNCTION(BlueprintCallable, Category = "Climate")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    /** Get humidity at a world position */
    UFUNCTION(BlueprintCallable, Category = "Climate")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    /** Get danger level at a world position */
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    float GetDangerLevelAtLocation(FVector WorldLocation) const;

    /** Initialize all biome definitions with default prehistoric data */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biomes")
    void InitializeBiomes();

    /** Debug: log biome data at player location */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugLogBiomeAtPlayerLocation();

protected:
    virtual void BeginPlay() override;

private:
    /** Simple 2D noise for biome placement */
    float BiomeNoise(float X, float Y, int32 Seed) const;

    /** Map noise value to biome type */
    EEng_BiomeType NoiseValueToBiome(float NoiseValue) const;

    void SetupDefaultBiomeDefinitions();
};
