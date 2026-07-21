#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "SharedTypes.h"
#include "World_BiomeClimateSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_ClimateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Temperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Humidity = 0.6f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float WindSpeed = 5.0f; // m/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float AtmosphericPressure = 101325.0f; // Pascal

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Precipitation = 0.0f; // mm/hour

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float CloudCover = 0.3f; // 0.0 to 1.0

    FWorld_ClimateData()
    {
        Temperature = 25.0f;
        Humidity = 0.6f;
        WindSpeed = 5.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        AtmosphericPressure = 101325.0f;
        Precipitation = 0.0f;
        CloudCover = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeClimateProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Climate")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Climate")
    FWorld_ClimateData BaseClimate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Climate")
    FWorld_ClimateData SeasonalVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Climate")
    float TemperatureFluctuationRange = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Climate")
    float HumidityFluctuationRange = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Climate")
    float WindVariability = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> WindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> RainSound;

    FWorld_BiomeClimateProfile()
    {
        BiomeType = EBiomeType::Forest;
        TemperatureFluctuationRange = 5.0f;
        HumidityFluctuationRange = 0.2f;
        WindVariability = 2.0f;
    }
};

/**
 * Manages climate conditions for different biomes in the Cretaceous world
 * Provides realistic temperature, humidity, wind patterns for each biome type
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeClimateSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeClimateSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Climate Management
    UFUNCTION(BlueprintCallable, Category = "Climate")
    FWorld_ClimateData GetCurrentClimate(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Climate")
    void UpdateClimateForBiome(EBiomeType BiomeType, const FWorld_ClimateData& NewClimate);

    UFUNCTION(BlueprintCallable, Category = "Climate")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Climate")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Climate")
    FVector GetWindAtLocation(const FVector& Location) const;

    // Biome Climate Profiles
    UFUNCTION(BlueprintCallable, Category = "Biome Climate")
    void InitializeBiomeClimateProfiles();

    UFUNCTION(BlueprintCallable, Category = "Biome Climate")
    FWorld_BiomeClimateProfile GetBiomeClimateProfile(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Climate")
    void SetBiomeClimateProfile(EBiomeType BiomeType, const FWorld_BiomeClimateProfile& Profile);

    // Weather Effects
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TriggerRainfall(EBiomeType BiomeType, float Intensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TriggerWindStorm(EBiomeType BiomeType, float WindSpeed, const FVector& Direction);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ClearWeather(EBiomeType BiomeType);

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateBiomeAudio(EBiomeType BiomeType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayClimateAudio(const FWorld_ClimateData& Climate, const FVector& Location);

    // Time of Day Effects
    UFUNCTION(BlueprintCallable, Category = "Time")
    void UpdateDailyClimateVariation(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Time")
    float GetDailyTemperatureModifier(float TimeOfDay) const;

    // Seasonal Changes
    UFUNCTION(BlueprintCallable, Category = "Seasonal")
    void UpdateSeasonalClimate(float SeasonProgress);

    UFUNCTION(BlueprintCallable, Category = "Seasonal")
    FWorld_ClimateData ApplySeasonalModifiers(const FWorld_ClimateData& BaseClimate, float SeasonProgress) const;

private:
    // Climate Data Storage
    UPROPERTY(EditAnywhere, Category = "Climate Data")
    TMap<EBiomeType, FWorld_BiomeClimateProfile> BiomeClimateProfiles;

    UPROPERTY(EditAnywhere, Category = "Climate Data")
    TMap<EBiomeType, FWorld_ClimateData> CurrentBiomeClimates;

    // Audio Components
    UPROPERTY(VisibleAnywhere, Category = "Audio")
    TMap<EBiomeType, UAudioComponent*> BiomeAudioComponents;

    // Climate Parameters
    UPROPERTY(EditAnywhere, Category = "Climate Settings")
    float ClimateUpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Climate Settings")
    float TemperatureVariationSpeed = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Climate Settings")
    float HumidityVariationSpeed = 0.05f;

    UPROPERTY(EditAnywhere, Category = "Climate Settings")
    float WindVariationSpeed = 0.2f;

    // Time Tracking
    float ClimateUpdateTimer = 0.0f;
    float CurrentTimeOfDay = 12.0f; // 0-24 hours
    float CurrentSeasonProgress = 0.25f; // 0-1 (0=winter, 0.25=spring, 0.5=summer, 0.75=autumn)

    // Helper Functions
    EBiomeType GetBiomeTypeAtLocation(const FVector& Location) const;
    FWorld_ClimateData InterpolateClimate(const FWorld_ClimateData& ClimateA, const FWorld_ClimateData& ClimateB, float Alpha) const;
    void UpdateClimateVariations(float DeltaTime);
    void CreateBiomeAudioComponent(EBiomeType BiomeType);
};