#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate", meta = (ClampMin = "-50.0", ClampMax = "60.0"))
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ElevationMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ElevationMax = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainfallChance = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float DinosaurSpawnMultiplier = 1.0f;

    FEng_BiomeData()
    {
        BiomeType = EEng_BiomeType::Forest;
        Temperature = 20.0f;
        Humidity = 50.0f;
        VegetationDensity = 0.5f;
        ElevationMin = 0.0f;
        ElevationMax = 1000.0f;
        RainfallChance = 0.3f;
        DinosaurSpawnMultiplier = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EEng_BiomeType FromBiome = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EEng_BiomeType ToBiome = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float TransitionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendStrength = 0.5f;

    FEng_BiomeTransition()
    {
        FromBiome = EEng_BiomeType::Forest;
        ToBiome = EEng_BiomeType::Grassland;
        TransitionDistance = 500.0f;
        BlendStrength = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core biome functions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    // Biome transition system
    UFUNCTION(BlueprintCallable, Category = "Biome Transitions")
    void RegisterBiomeTransition(const FEng_BiomeTransition& Transition);

    UFUNCTION(BlueprintCallable, Category = "Biome Transitions")
    FEng_BiomeData GetBlendedBiomeDataAtLocation(const FVector& WorldLocation) const;

    // Weather integration
    UFUNCTION(BlueprintCallable, Category = "Weather")
    bool ShouldRainAtLocation(const FVector& WorldLocation, float CurrentTime) const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    float GetWindStrengthAtLocation(const FVector& WorldLocation) const;

    // Dinosaur spawn integration
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Spawning")
    float GetDinosaurSpawnRateAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Spawning")
    TArray<EEng_DinosaurSpecies> GetValidDinosaurSpeciesForBiome(EEng_BiomeType BiomeType) const;

    // Performance and LOD
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetBiomeLODDistance(float HighDetailDistance, float MediumDetailDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetBiomeLODLevel(const FVector& WorldLocation, const FVector& ViewerLocation) const;

    // Debug and editor tools
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugDrawBiomeAtLocation(const FVector& WorldLocation, float Radius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBiomeSystemStatus() const;

protected:
    // Biome configuration data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeDataMap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "1000.0", ClampMax = "10000.0"))
    float MediumDetailDistance = 5000.0f;

    // World generation integration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Generation")
    float BiomeNoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Generation")
    int32 BiomeNoiseSeed = 12345;

private:
    // Internal helper functions
    float CalculateBiomeNoise(const FVector& WorldLocation) const;
    EEng_BiomeType DetermineBiomeFromNoise(float NoiseValue, float Elevation) const;
    float CalculateTransitionBlend(const FVector& WorldLocation, EEng_BiomeType BiomeA, EEng_BiomeType BiomeB) const;
    
    // Cache for performance
    mutable TMap<FIntVector, EEng_BiomeType> BiomeCache;
    mutable float LastCacheUpdateTime = 0.0f;
    static constexpr float CacheUpdateInterval = 1.0f;
    static constexpr int32 CacheGridSize = 1000; // 1km grid cells
    
    void InitializeDefaultBiomeData();
    void ClearBiomeCache() const;
};