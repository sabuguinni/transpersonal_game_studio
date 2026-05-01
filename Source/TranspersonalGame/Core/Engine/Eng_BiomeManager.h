#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

/**
 * BIOME MANAGER - CORE ARCHITECTURE SYSTEM
 * Engine Architect Agent #02
 * 
 * Manages biome generation, transition zones, and environmental parameters.
 * This is the core system that defines how the prehistoric world is structured.
 * 
 * BIOMES SUPPORTED:
 * - Grassland: Open plains with scattered trees
 * - Forest: Dense vegetation, limited visibility
 * - Desert: Arid environment, extreme temperatures
 * - Swamp: Wetland areas, dangerous terrain
 * - Mountains: High elevation, rocky terrain
 * - River/Lake: Water bodies and shorelines
 * - Coast: Ocean boundaries
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    FEng_BiomeParameters()
    {
        BiomeType = EEng_BiomeType::Grassland;
        Temperature = 20.0f;
        Humidity = 0.5f;
        VegetationDensity = 0.5f;
        RockDensity = 0.3f;
        WaterLevel = 0.0f;
        FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
        FogDensity = 0.02f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EEng_BiomeType FromBiome = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EEng_BiomeType ToBiome = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;

    FEng_BiomeTransition()
    {
        FromBiome = EEng_BiomeType::Grassland;
        ToBiome = EEng_BiomeType::Forest;
        TransitionDistance = 1000.0f;
        BlendFactor = 0.5f;
    }
};

/**
 * Biome Manager Subsystem
 * Handles biome generation, environmental parameters, and transition zones
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core biome functions
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeParameters GetBiomeParameters(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeParameters GetBlendedBiomeParameters(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SetBiomeParameters(EEng_BiomeType BiomeType, const FEng_BiomeParameters& Parameters);

    // Transition zone management
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsInTransitionZone(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeTransition GetTransitionData(const FVector& WorldLocation) const;

    // Environmental queries
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    // Debug and validation
    UFUNCTION(BlueprintCallable, Category = "Biome", CallInEditor)
    void ValidateBiomeConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Biome", CallInEditor)
    void GenerateDebugBiomeMap();

protected:
    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EEng_BiomeType, FEng_BiomeParameters> BiomeParametersMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    // World parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float WorldSizeKm = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float BiomeNoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float TransitionZoneSize = 500.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableBiomeBlending = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float BiomeUpdateFrequency = 1.0f;

private:
    // Internal helper functions
    float CalculateNoiseValue(const FVector& Location, float Scale) const;
    EEng_BiomeType DetermineBiomeFromNoise(float NoiseValue, float Elevation) const;
    FEng_BiomeParameters BlendBiomeParameters(const FEng_BiomeParameters& BiomeA, const FEng_BiomeParameters& BiomeB, float BlendFactor) const;
    
    // Cache for performance
    mutable TMap<FIntPoint, EEng_BiomeType> BiomeCache;
    mutable float LastCacheUpdate = 0.0f;
};