#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

// Forward declarations
class UBiomeComponent;
class ALandscape;
class UStaticMeshComponent;

/**
 * Biome data structure for data table configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    EEng_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temperature")
    float TemperatureMin;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temperature")
    float TemperatureMax;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Humidity")
    float HumidityMin;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Humidity")
    float HumidityMax;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vegetation")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wildlife")
    TArray<TSubclassOf<APawn>> CommonSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
    TArray<FString> AvailableResources;

    FEng_BiomeData()
    {
        BiomeType = EEng_BiomeType::Grassland;
        BiomeName = TEXT("Default Biome");
        TemperatureMin = 15.0f;
        TemperatureMax = 25.0f;
        HumidityMin = 0.3f;
        HumidityMax = 0.7f;
        VegetationDensity = 0.5f;
    }
};

/**
 * Biome transition zone configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    EEng_BiomeType FromBiome;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    EEng_BiomeType ToBiome;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    float TransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
    float BlendStrength;

    FEng_BiomeTransition()
    {
        FromBiome = EEng_BiomeType::Grassland;
        ToBiome = EEng_BiomeType::Forest;
        TransitionDistance = 1000.0f;
        BlendStrength = 0.5f;
    }
};

/**
 * Central biome management system for the prehistoric world
 * Handles biome definition, transitions, and environmental parameters
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UBiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core biome data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Data", meta = (AllowPrivateAccess = "true"))
    UDataTable* BiomeDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Configuration", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_BiomeData> BiomeTypes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Configuration", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_BiomeTransition> BiomeTransitions;

    // World parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Parameters", meta = (AllowPrivateAccess = "true"))
    float WorldTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Parameters", meta = (AllowPrivateAccess = "true"))
    float WorldHumidity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Parameters", meta = (AllowPrivateAccess = "true"))
    float SeasonalVariation;

    // Biome sampling and noise
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    float BiomeNoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    int32 BiomeNoiseSeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    float BiomeBlendRadius;

public:
    // Core biome functions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeInfluence(const FVector& WorldLocation, EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EEng_BiomeType> GetNearbyBiomes(const FVector& WorldLocation, float Radius) const;

    // Environmental parameters
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    // Biome transitions
    UFUNCTION(BlueprintCallable, Category = "Biome Transitions")
    bool IsBiomeTransitionZone(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Transitions")
    FEng_BiomeTransition GetBiomeTransition(const FVector& WorldLocation) const;

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RegisterBiomeComponent(UBiomeComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void UnregisterBiomeComponent(UBiomeComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RefreshBiomeData();

    // Debug and validation
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ValidateBiomeConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void GenerateBiomePreview();

protected:
    // Internal biome calculation
    float CalculateBiomeNoise(const FVector& WorldLocation, int32 Octaves = 3) const;
    float CalculateTemperatureNoise(const FVector& WorldLocation) const;
    float CalculateHumidityNoise(const FVector& WorldLocation) const;

    // Biome component tracking
    UPROPERTY()
    TArray<UBiomeComponent*> RegisteredComponents;

    // Cached biome data for performance
    UPROPERTY()
    TMap<EEng_BiomeType, FEng_BiomeData> CachedBiomeData;

    // Validation flags
    UPROPERTY()
    bool bBiomeDataValidated;

    UPROPERTY()
    bool bTransitionDataValidated;
};