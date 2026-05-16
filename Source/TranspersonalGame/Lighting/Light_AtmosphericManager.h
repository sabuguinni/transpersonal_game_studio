#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_AtmosphericManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float MieScattering = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    FLight_AtmosphericSettings()
    {
        // Default Cretaceous tropical settings
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Current atmospheric state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_AtmosphericSettings AtmosphericSettings;

    // Day/Night cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableDayNightCycle = true;

    // References to lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class APostProcessVolume* PostProcessVolume;

public:
    virtual void Tick(float DeltaTime) override;

    // Atmospheric control functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeatherType(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetCurrentHour(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ConfigureTropicalLighting();

    // Weather transition functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ApplyStormLighting();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ApplyClearWeatherLighting();

protected:
    // Internal update functions
    void FindLightingActors();
    void UpdateDayNightCycle(float DeltaTime);
    void InterpolateLightingSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha);
    float CalculateSunAngle() const;
    FRotator CalculateSunRotation() const;

private:
    // Weather transition state
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 5.0f;
    ELight_WeatherType TargetWeatherType;
    FLight_AtmosphericSettings StartSettings;
    FLight_AtmosphericSettings TargetSettings;

    // Presets for different weather types
    FLight_AtmosphericSettings ClearWeatherSettings;
    FLight_AtmosphericSettings CloudyWeatherSettings;
    FLight_AtmosphericSettings StormWeatherSettings;
    FLight_AtmosphericSettings FoggyWeatherSettings;

    void InitializeWeatherPresets();
    FLight_AtmosphericSettings GetSettingsForWeather(ELight_WeatherType Weather) const;
};