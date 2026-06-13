#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    TropicalForest      UMETA(DisplayName = "Tropical Forest"),
    TemperateGrassland  UMETA(DisplayName = "Temperate Grassland"),
    DesertOasis         UMETA(DisplayName = "Desert Oasis"),
    RockyHighlands      UMETA(DisplayName = "Rocky Highlands"),
    SwampLands          UMETA(DisplayName = "Swamp Lands"),
    Transitional        UMETA(DisplayName = "Transitional Zone")
};

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear               UMETA(DisplayName = "Clear"),
    Cloudy              UMETA(DisplayName = "Cloudy"),
    Rainy               UMETA(DisplayName = "Rainy"),
    Stormy              UMETA(DisplayName = "Stormy"),
    Foggy               UMETA(DisplayName = "Foggy"),
    Windy               UMETA(DisplayName = "Windy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::TropicalForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EWorld_WeatherType> PossibleWeather;

    FWorld_BiomeData()
    {
        PossibleWeather.Add(EWorld_WeatherType::Clear);
        PossibleWeather.Add(EWorld_WeatherType::Cloudy);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType CurrentWeather = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TimeRemaining = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Visibility = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeInfluence(const FVector& Location, EWorld_BiomeType BiomeType) const;

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateWeatherSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void ChangeWeather(EWorld_WeatherType NewWeather, float Intensity = 1.0f, float Duration = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    FWorld_WeatherState GetCurrentWeather() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void ApplyWeatherEffects();

    // Environmental Queries
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FVector GetWindAtLocation(const FVector& Location) const;

    // Seasonal System
    UFUNCTION(BlueprintCallable, Category = "Seasons")
    void UpdateSeasonalEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Seasons")
    void SetSeason(float SeasonValue); // 0-1 representing year cycle

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWorld_WeatherState CurrentWeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeInterval = 600.0f; // 10 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons")
    float CurrentSeason = 0.0f; // 0-1 representing year cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasons")
    float SeasonalSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float GlobalTemperatureModifier = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float GlobalHumidityModifier = 0.0f;

private:
    float WeatherTimer = 0.0f;
    float SeasonTimer = 0.0f;

    void SetupDefaultBiomes();
    EWorld_WeatherType SelectRandomWeatherForBiome(EWorld_BiomeType BiomeType) const;
    void ApplyFogEffects(float Intensity);
    void ApplyWindEffects(const FVector& WindDirection, float Strength);
};