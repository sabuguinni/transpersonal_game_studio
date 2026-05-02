#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"), 
    Mountain    UMETA(DisplayName = "Mountain"),
    Desert      UMETA(DisplayName = "Desert"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Transition  UMETA(DisplayName = "Transition")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureBase = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityBase = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> WeatherEffects;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Forest;
        CenterLocation = FVector::ZeroVector;
        Radius = 2000.0f;
        TemperatureBase = 20.0f;
        HumidityBase = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_WeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.0f;

    FWorld_WeatherData()
    {
        Temperature = 20.0f;
        Humidity = 50.0f;
        WindSpeed = 0.0f;
        RainIntensity = 0.0f;
        FogDensity = 0.0f;
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

public:
    virtual void Tick(float DeltaTime) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_WeatherData GetCurrentWeather(const FVector& Location) const;

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentalAudio(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateWeatherEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SpawnBiomeActors(EWorld_BiomeType BiomeType, const FVector& Location);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeDataArray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWorld_WeatherData CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float EnvironmentUpdateInterval = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float LastEnvironmentUpdate = 0.0f;

private:
    void SetupDefaultBiomes();
    float CalculateDistanceToBiome(const FVector& Location, const FWorld_BiomeData& BiomeData) const;
    FWorld_WeatherData InterpolateWeather(const FWorld_WeatherData& WeatherA, const FWorld_WeatherData& WeatherB, float Alpha) const;
};