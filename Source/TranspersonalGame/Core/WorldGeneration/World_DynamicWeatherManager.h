#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "SharedTypes.h"
#include "World_DynamicWeatherManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear Skies"),
    Overcast        UMETA(DisplayName = "Overcast"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm    UMETA(DisplayName = "Thunderstorm"),
    Fog             UMETA(DisplayName = "Fog"),
    Sandstorm       UMETA(DisplayName = "Sandstorm"),
    Snow            UMETA(DisplayName = "Snow"),
    Blizzard        UMETA(DisplayName = "Blizzard"),
    HeatWave        UMETA(DisplayName = "Heat Wave")
};

UENUM(BlueprintType)
enum class EWorld_SeasonType : uint8
{
    WetSeason       UMETA(DisplayName = "Wet Season"),
    DrySeason       UMETA(DisplayName = "Dry Season"),
    CoolSeason      UMETA(DisplayName = "Cool Season"),
    HotSeason       UMETA(DisplayName = "Hot Season")
};

USTRUCT(BlueprintType)
struct FWorld_WeatherZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Zone")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Zone")
    float ZoneRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Zone")
    EWorld_WeatherType CurrentWeather = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Zone")
    float WeatherIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Zone")
    float TransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Zone")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FWorld_AtmosphericData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float Pressure = 1013.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float WindSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float Visibility = 10000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_DynamicWeatherManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_DynamicWeatherManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Weather Zone Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Zones")
    TArray<FWorld_WeatherZoneData> WeatherZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    EWorld_SeasonType CurrentSeason = EWorld_SeasonType::WetSeason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float SeasonProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float DayNightCycle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FWorld_AtmosphericData GlobalAtmosphere;

    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeatherInZone(int32 ZoneIndex, EWorld_WeatherType NewWeather, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void TransitionToSeason(EWorld_SeasonType NewSeason, float TransitionTime = 60.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void UpdateDayNightCycle(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetGlobalWindPattern(FVector Direction, float Speed);

    // Weather Query Functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weather Query")
    EWorld_WeatherType GetWeatherAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weather Query")
    FWorld_AtmosphericData GetAtmosphericDataAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weather Query")
    float GetWeatherIntensityAtLocation(FVector Location) const;

    // Weather Effects
    UFUNCTION(BlueprintCallable, Category = "Weather Effects")
    void SpawnWeatherParticles(FVector Location, EWorld_WeatherType WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather Effects")
    void UpdateLightingForWeather(EWorld_WeatherType WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather Effects")
    void ApplyWeatherToFoliage(EWorld_WeatherType WeatherType, float Intensity);

protected:
    // Internal weather update functions
    void UpdateWeatherZones(float DeltaTime);
    void UpdateSeasonalTransitions(float DeltaTime);
    void UpdateAtmosphericPressure(float DeltaTime);
    void UpdateWindPatterns(float DeltaTime);

    // Weather zone components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* WeatherDetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* GlobalWeatherParticles;

    // Weather transition timers
    float SeasonTransitionTimer = 0.0f;
    float WeatherUpdateTimer = 0.0f;
    float AtmosphericUpdateTimer = 0.0f;

    // Weather intensity curves
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Curves")
    class UCurveFloat* TemperatureCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Curves")
    class UCurveFloat* HumidityCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Curves")
    class UCurveFloat* WindSpeedCurve;
};