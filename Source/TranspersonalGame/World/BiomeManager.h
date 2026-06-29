#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Jungle          UMETA(DisplayName = "Jungle"),
    Savanna         UMETA(DisplayName = "Savanna"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    CoastalPlain    UMETA(DisplayName = "Coastal Plain"),
    RiverDelta      UMETA(DisplayName = "River Delta"),
    Count           UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Savanna");

    /** Ambient temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float TemperatureCelsius = 28.0f;

    /** Rainfall in mm/year */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float AnnualRainfallMM = 800.0f;

    /** Humidity 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float Humidity = 0.5f;

    /** Fog density 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.02f;

    /** Sun intensity multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float SunIntensityMultiplier = 1.0f;

    /** Ambient color tint for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor AmbientColorTint = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    /** Vegetation density 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Ecology")
    float VegetationDensity = 0.6f;

    /** Predator spawn weight (relative) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Ecology")
    float PredatorSpawnWeight = 1.0f;

    /** Herbivore spawn weight (relative) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Ecology")
    float HerbivoreSpawnWeight = 1.5f;

    /** World-space bounds of this biome region */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|World")
    FBox BiomeBounds = FBox(FVector(-5000, -5000, -100), FVector(5000, 5000, 2000));
};

/**
 * ABiomeManager — manages biome regions, transitions, and environmental properties.
 * Placed once in the level. Other systems query it for biome data at any world location.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (TranspersonalGame))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** All biome regions defined for this level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FEng_BiomeData> BiomeRegions;

    /** Transition blend distance in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float TransitionBlendDistance = 5000.0f;

    /** Returns the dominant biome at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Returns the full biome data struct at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FEng_BiomeData GetBiomeDataAtLocation(FVector WorldLocation) const;

    /** Returns blended biome data at a location (handles transitions) */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FEng_BiomeData GetBlendedBiomeData(FVector WorldLocation) const;

    /** Returns true if the location is within the given biome type */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool IsLocationInBiome(FVector WorldLocation, EEng_BiomeType BiomeType) const;

    /** Registers a new biome region at runtime */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void RegisterBiomeRegion(const FEng_BiomeData& NewBiome);

    /** Debug: draw all biome bounds in viewport */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biomes|Debug")
    void DrawBiomeBounds();

    /** Singleton accessor — returns the first ABiomeManager in the world */
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    static ABiomeManager* GetInstance(UObject* WorldContextObject);

protected:
    /** Initialises default biome regions if none are configured */
    void InitialiseDefaultBiomes();

    /** Finds the nearest biome to a world location */
    int32 FindNearestBiomeIndex(FVector WorldLocation) const;

private:
    /** Cached singleton reference */
    static ABiomeManager* CachedInstance;
};
