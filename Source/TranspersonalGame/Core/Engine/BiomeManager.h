#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * Biome System Architecture - Core biome management and world generation integration
 * Manages biome definitions, transitions, and environmental parameters
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core biome management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RegisterBiomeZone(const FEng_BiomeZone& BiomeZone);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FEng_BiomeZone> GetBiomeZonesInRadius(const FVector& Center, float Radius) const;

    // Environmental parameters
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    EEng_WeatherType GetWeatherAtLocation(const FVector& WorldLocation) const;

    // Biome transitions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsInBiomeTransitionZone(const FVector& WorldLocation, float TransitionRadius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeTransition CalculateBiomeTransition(const FVector& WorldLocation) const;

    // Debug and validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateBiomeConfiguration();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void GenerateDebugBiomeMap();

protected:
    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    TArray<FEng_BiomeZone> ActiveBiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float BiomeTransitionSmoothness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    int32 BiomeResolution = 100; // Units per biome sample

    // Environmental parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    FEng_EnvironmentalParameters GlobalEnvironmentalParams;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float TemperatureVariation = 15.0f; // Celsius variation

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float HumidityVariation = 0.4f; // 0-1 range variation

private:
    // Internal biome calculation
    EEng_BiomeType CalculateBiomeFromParameters(float Temperature, float Humidity, float Elevation) const;
    
    // Noise generation for environmental variation
    float GenerateTemperatureNoise(const FVector& Location) const;
    float GenerateHumidityNoise(const FVector& Location) const;
    
    // Biome zone management
    void InitializeDefaultBiomes();
    void UpdateBiomeZoneCache();
    
    // Cache for performance
    mutable TMap<FIntVector, EEng_BiomeType> BiomeCache;
    mutable TMap<FIntVector, FEng_EnvironmentalParameters> EnvironmentCache;
    
    // Configuration validation
    bool ValidateBiomeData(const FEng_BiomeData& BiomeData) const;
    void LogBiomeSystemStatus() const;
};