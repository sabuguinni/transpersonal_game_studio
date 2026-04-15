#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "Sound/AmbientSound.h"
#include "../SharedTypes.h"
#include "WorldGenBiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"), 
    Mountains   UMETA(DisplayName = "Mountains"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Swamp       UMETA(DisplayName = "Swamp")
};

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Snow        UMETA(DisplayName = "Snow"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString AmbientSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString ReverbSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasDinosaurs = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FString WeatherSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAffectsVisibility = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float VisibilityReduction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAffectsTemperature = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TemperatureChange = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorldGenBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorldGenBiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiome(const FWorld_BiomeConfig& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeConfig GetBiomeConfig(EWorld_BiomeType BiomeType) const;

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EWorld_WeatherType WeatherType, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartWeatherTransition(EWorld_WeatherType NewWeather, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EWorld_WeatherType GetCurrentWeather() const { return CurrentWeather; }

    // Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateBiomeAudio(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayWeatherAudio(EWorld_WeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopWeatherAudio();

    // Terrain Generation
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void GenerateTerrainHeightmap();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetTerrainHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void ApplyBiomeBlending(const FVector& Location, float BlendRadius);

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WeatherAudioComponent;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    bool bUseNoiseForBiomeBlending = true;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType CurrentWeather = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<FWorld_WeatherConfig> WeatherConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeInterval = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Audio System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AudioUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxAudioDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bUseAudioOcclusion = true;

    // Terrain Generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 TerrainResolution = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float TerrainScale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 NoiseOctaves = 4;

private:
    // Internal state
    float LastAudioUpdateTime = 0.0f;
    float LastWeatherUpdateTime = 0.0f;
    EWorld_BiomeType LastPlayerBiome = EWorld_BiomeType::Forest;
    
    // Helper functions
    float CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeConfig& Biome) const;
    void UpdateWeatherSystem(float DeltaTime);
    void BlendBiomeAudio(EWorld_BiomeType PrimaryBiome, EWorld_BiomeType SecondaryBiome, float BlendFactor);
};