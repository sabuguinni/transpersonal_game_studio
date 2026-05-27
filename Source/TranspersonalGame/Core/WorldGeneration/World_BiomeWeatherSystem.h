#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "SharedTypes.h"
#include "World_BiomeWeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"), 
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Sandstorm   UMETA(DisplayName = "Sandstorm")
};

UENUM(BlueprintType)
enum class EWorld_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f; // 5 minutes default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay = 12.0f; // 0-24 hour format

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDuration = 1200.0f; // 20 minutes real time = 24 hours game time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor MoonColor = FLinearColor(0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float AtmosphereThickness = 1.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeWeatherProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    TArray<EWorld_WeatherType> CommonWeatherTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    float WeatherChangeFrequency = 0.1f; // Chance per minute

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    float TemperatureBase = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    float HumidityBase = 0.5f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    FLinearColor AmbientColor = FLinearColor(0.3f, 0.3f, 0.4f);
};

/**
 * Manages dynamic weather and day/night cycle for different biomes
 * Integrates with lighting and atmosphere systems for realistic environmental effects
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeWeatherSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeWeatherSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather Control
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeather(EWorld_WeatherType NewWeather, float Intensity = 1.0f, float Duration = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    EWorld_WeatherType GetCurrentWeather() const { return CurrentWeather.WeatherType; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    float GetCurrentTimeOfDay() const { return CurrentTime.TimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    EWorld_TimeOfDay GetTimeOfDayEnum() const;

    // Weather Events
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TriggerRandomWeather();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeather(EWorld_WeatherType TargetWeather, float TransitionDuration = 30.0f);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    float GetTemperature() const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    float GetHumidity() const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    float GetWindStrength() const { return CurrentWeather.WindStrength; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    bool IsRaining() const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    bool IsNight() const;

    // Editor Tools
    UFUNCTION(CallInEditor, Category = "Weather System")
    void InitializeBiomeProfiles();

    UFUNCTION(CallInEditor, Category = "Weather System")
    void PreviewWeather();

    UFUNCTION(CallInEditor, Category = "Weather System")
    void ResetToDefault();

protected:
    // Core Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FWorld_WeatherSettings CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    FWorld_TimeSettings CurrentTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    EBiomeType CurrentBiome = EBiomeType::Savana;

    // Biome Profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Profiles")
    TArray<FWorld_BiomeWeatherProfile> BiomeProfiles;

    // Component References
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    class AExponentialHeightFog* HeightFog;

    // Weather Transition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    bool bIsTransitioning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    FWorld_WeatherSettings TargetWeather;

    // Timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timers")
    float WeatherTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timers")
    float TimeUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timers")
    float LastTimeUpdate = 0.0f;

private:
    // Internal Methods
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherEffects(float DeltaTime);
    void UpdateLighting();
    void UpdateAtmosphere();
    void UpdateFog();
    void ProcessWeatherTransition(float DeltaTime);
    void FindLightingComponents();
    FWorld_BiomeWeatherProfile* GetCurrentBiomeProfile();
    void ApplyWeatherEffects();
    float CalculateTemperature() const;
    float CalculateHumidity() const;
    FLinearColor GetSkyColorForTime() const;
    float GetSunAngleForTime() const;
    void LogWeatherChange() const;
};