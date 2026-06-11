#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temperature")
    float MinTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temperature")
    float MaxTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Humidity")
    float HumidityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")
    float TerrainRoughness;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wildlife")
    TArray<EDinosaurSpecies> NativeDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    TArray<EResourceType> AvailableResources;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    TArray<EWeatherType> PossibleWeather;

    FEng_BiomeData()
    {
        BiomeType = EBiomeType::Forest;
        BiomeName = TEXT("Unknown Biome");
        MinTemperature = 15.0f;
        MaxTemperature = 25.0f;
        HumidityLevel = 0.5f;
        VegetationDensity = 0.5f;
        TerrainRoughness = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    EBiomeType FromBiome;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    EBiomeType ToBiome;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    float TransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    float BlendFactor;

    FEng_BiomeTransition()
    {
        FromBiome = EBiomeType::Forest;
        ToBiome = EBiomeType::Plains;
        TransitionDistance = 1000.0f;
        BlendFactor = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome query functions
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    // Biome management functions
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SetBiomeAtLocation(const FVector& WorldLocation, EBiomeType NewBiome, float Radius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<EDinosaurSpecies> GetNativeDinosaursForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<EResourceType> GetAvailableResourcesForBiome(EBiomeType BiomeType) const;

    // Transition functions
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsInTransitionZone(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeTransition GetTransitionData(const FVector& WorldLocation) const;

    // Weather integration
    UFUNCTION(BlueprintCallable, Category = "Weather")
    TArray<EWeatherType> GetPossibleWeatherForBiome(EBiomeType BiomeType) const;

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugDrawBiomeMap();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ValidateBiomeConfiguration();

protected:
    // Biome data table
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    UDataTable* BiomeDataTable;

    // Biome map resolution (units per pixel)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float BiomeMapResolution;

    // World bounds for biome system
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    FVector2D WorldBounds;

    // Transition zone width
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float TransitionZoneWidth;

private:
    // Internal biome map (2D grid)
    TArray<TArray<EBiomeType>> BiomeMap;

    // Cached biome data
    TMap<EBiomeType, FEng_BiomeData> BiomeDataCache;

    // Transition zones
    TArray<FEng_BiomeTransition> ActiveTransitions;

    // Helper functions
    FVector2D WorldToMapCoordinates(const FVector& WorldLocation) const;
    FVector MapToWorldCoordinates(const FVector2D& MapCoordinates) const;
    void InitializeBiomeMap();
    void CacheBiomeData();
    EBiomeType SampleBiomeFromMap(const FVector2D& MapCoordinates) const;
    float CalculateTransitionBlend(const FVector& WorldLocation, const FEng_BiomeTransition& Transition) const;
};