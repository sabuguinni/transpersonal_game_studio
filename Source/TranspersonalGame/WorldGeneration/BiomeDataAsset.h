#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "BiomeDataAsset.generated.h"

UENUM(BlueprintType)
enum class EPrehistoricBiome : uint8
{
    DenseForest      UMETA(DisplayName = "Dense Prehistoric Forest"),
    OpenPlains       UMETA(DisplayName = "Prehistoric Plains"),
    RockyHills       UMETA(DisplayName = "Rocky Highlands"),
    RiverValley      UMETA(DisplayName = "River Valley"),
    SwampLands       UMETA(DisplayName = "Prehistoric Swamplands"),
    VolcanicRegion   UMETA(DisplayName = "Volcanic Region"),
    CoastalArea      UMETA(DisplayName = "Coastal Region"),
    CaveSystem       UMETA(DisplayName = "Cave Networks")
};

USTRUCT(BlueprintType)
struct FVegetationSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FString VegetationName = "Unknown Plant";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float SpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float MinScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float MaxScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bIsEdible = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bIsPoisonous = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float NutritionalValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FString> RequiredTools; // Tools needed to harvest

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector PreferredSlope = FVector(0.0f, 30.0f, 0.0f); // Min, Max, Optimal slope angles
};

USTRUCT(BlueprintType)
struct FRockFormationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    FString FormationName = "Rock Formation";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    float SpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    float MinScale = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    float MaxScale = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    bool bCanBeClimbed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    bool bProvidesResources = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    TArray<FString> ResourceTypes; // Stone, Metal, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocks")
    float HarvestDifficulty = 1.0f;
};

USTRUCT(BlueprintType)
struct FBiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> WindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> WaterSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundCue>> AnimalSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SoundVariation = 0.3f;
};

USTRUCT(BlueprintType)
struct FBiomeEffectsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<UNiagaraSystem> FogEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<UNiagaraSystem> ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<UNiagaraSystem> WeatherEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float FogDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float EffectIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FBiomeClimateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float BaseTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float TemperatureVariation = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Humidity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float RainfallProbability = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    bool bHasSeasonalChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float DayNightTemperatureDifference = 10.0f;
};

USTRUCT(BlueprintType)
struct FBiomeDangerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    float PredatorSpawnRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    TArray<FString> CommonPredators;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    float EnvironmentalHazardLevel = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    TArray<FString> HazardTypes; // Poisonous plants, unstable ground, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    float VisibilityModifier = 1.0f; // How well player can see threats

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    bool bHasQuicksand = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    bool bHasPoisonousGas = false;
};

/**
 * Biome Data Asset - Defines all characteristics of a prehistoric biome
 * Contains vegetation, climate, audio, visual effects, and gameplay data
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBiomeDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UBiomeDataAsset();

    // === BIOME IDENTIFICATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Info")
    EPrehistoricBiome BiomeType = EPrehistoricBiome::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Info")
    FString BiomeName = "Unnamed Biome";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Info")
    FText BiomeDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Info")
    TSoftObjectPtr<UTexture2D> BiomeIcon;

    // === TERRAIN CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float BaseElevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ElevationVariation = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float SlopePreference = 0.3f; // 0 = flat, 1 = steep

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<TSoftObjectPtr<UMaterialInterface>> TerrainLayerMaterials;

    // === VEGETATION DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FVegetationSpawnData> TreeSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FVegetationSpawnData> BushSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FVegetationSpawnData> GrassSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FVegetationSpawnData> FlowerSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationVariation = 0.3f;

    // === ROCK FORMATIONS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formations")
    TArray<FRockFormationData> RockFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formations")
    float RockDensity = 0.2f;

    // === CLIMATE DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    FBiomeClimateData ClimateSettings;

    // === AUDIO SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FBiomeAudioData AudioSettings;

    // === VISUAL EFFECTS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FBiomeEffectsData EffectsSettings;

    // === DANGER SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    FBiomeDangerData DangerSettings;

    // === SURVIVAL IMPACT ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float WaterAvailability = 0.5f; // 0 = no water, 1 = abundant

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FoodAvailability = 0.5f; // Edible plants and small game

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float ShelterMaterials = 0.5f; // Available building materials

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float NavigationDifficulty = 0.3f; // How hard to navigate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    TArray<FString> AvailableResources;

public:
    // === BLUEPRINT CALLABLE FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Biome Data")
    FVegetationSpawnData GetRandomTreeSpecies() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Data")
    FVegetationSpawnData GetRandomBushSpecies() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Data")
    FRockFormationData GetRandomRockFormation() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Data")
    float GetSurvivalDifficulty() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Data")
    bool IsResourceAvailable(const FString& ResourceName) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Data")
    TArray<FString> GetEdiblePlants() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Data")
    TArray<FString> GetPoisonousPlants() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Data")
    float GetTemperatureAtTime(float TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Data")
    bool ShouldSpawnWeatherEffect() const;

    // === VALIDATION ===
    
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif

private:
    // === INTERNAL UTILITIES ===
    
    template<typename T>
    T GetRandomFromArray(const TArray<T>& Array) const;
    
    void ValidateVegetationData();
    void ValidateRockFormationData();
    void ValidateClimateData();
};