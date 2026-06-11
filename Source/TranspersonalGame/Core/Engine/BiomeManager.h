#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * BiomeManager - Core biome system architecture
 * Manages biome generation, transitions, and environmental parameters
 * Engine Architect Agent #02 - Cycle 020
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core biome properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float TemperatureBase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float HumidityBase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float ElevationInfluence;

    // Biome generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 BiomeNoiseOctaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float BiomeNoiseFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float BiomeNoiseAmplitude;

    // Environmental effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float FoliageDensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WildlifeDensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float ResourceDensityMultiplier;

    // Biome transition system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionSmoothness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    bool bEnableGradualTransitions;

public:
    // Core biome functions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetElevationInfluenceAtLocation(FVector WorldLocation);

    // Biome generation functions
    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateBiomeMap(int32 MapSize, float WorldScale);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void RefreshBiomeData();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    bool ValidateBiomeConfiguration();

    // Environmental integration
    UFUNCTION(BlueprintCallable, Category = "Environment")
    TArray<FEng_FoliageSpawnData> GetFoliageForBiome(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    TArray<FEng_WildlifeSpawnData> GetWildlifeForBiome(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FEng_WeatherParameters GetWeatherForBiome(EEng_BiomeType BiomeType);

    // Transition system
    UFUNCTION(BlueprintCallable, Category = "Transitions")
    float CalculateBiomeTransition(FVector FromLocation, FVector ToLocation);

    UFUNCTION(BlueprintCallable, Category = "Transitions")
    void UpdateBiomeTransitions(float DeltaTime);

    // Debug and validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawBiomeMap();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogBiomeStatistics();

private:
    // Internal biome calculation
    float CalculateNoiseValue(FVector Location, int32 Octaves, float Frequency, float Amplitude);
    EEng_BiomeType DetermineBiomeFromParameters(float Temperature, float Humidity, float Elevation);
    void InitializeBiomeDefaults();
    void UpdateEnvironmentalEffects();

    // Cached biome data
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeDataCache;
    bool bBiomeDataInitialized;
    float LastUpdateTime;
};