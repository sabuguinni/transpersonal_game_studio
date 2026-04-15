#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"), 
    Mountain    UMETA(DisplayName = "Mountain"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Plains      UMETA(DisplayName = "Plains"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Coastal     UMETA(DisplayName = "Coastal")
};

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Windy       UMETA(DisplayName = "Windy")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown Biome");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WindStrength = 0.3f;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Forest;
        BiomeName = TEXT("Unknown Biome");
        Temperature = 20.0f;
        Humidity = 0.5f;
        WindStrength = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_WeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> WeatherSound;

    FWorld_WeatherData()
    {
        WeatherType = EWorld_WeatherType::Clear;
        Intensity = 0.5f;
        Duration = 300.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* BiomeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WeatherAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeData> AvailableBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    TArray<FWorld_WeatherData> AvailableWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FWorld_BiomeData CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FWorld_WeatherData CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BiomeAudioVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float WeatherAudioVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition Settings")
    float BiomeTransitionTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition Settings")
    float WeatherTransitionTime = 3.0f;

private:
    float CurrentTransitionTime = 0.0f;
    bool bIsTransitioning = false;
    FWorld_BiomeData TargetBiome;
    FWorld_WeatherData TargetWeather;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetCurrentBiome(EWorld_BiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetCurrentWeather(EWorld_WeatherType NewWeatherType, float NewIntensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetCurrentBiomeType() const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    EWorld_WeatherType GetCurrentWeatherType() const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetCurrentTemperature() const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetCurrentHumidity() const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetCurrentWindStrength() const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateBiomeAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateWeatherAudio();

    UFUNCTION(BlueprintCallable, Category = "Transition System")
    void StartBiomeTransition(EWorld_BiomeType TargetBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Transition System")
    void StartWeatherTransition(EWorld_WeatherType TargetWeatherType, float TargetIntensity = 0.5f);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnBiomeChanged(EWorld_BiomeType NewBiome);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnWeatherChanged(EWorld_WeatherType NewWeather, float Intensity);

    void InitializeDefaultBiomes();
    void InitializeDefaultWeather();
    void ProcessTransitions(float DeltaTime);
    FWorld_BiomeData* FindBiomeData(EWorld_BiomeType BiomeType);
    FWorld_WeatherData* FindWeatherData(EWorld_WeatherType WeatherType);
};