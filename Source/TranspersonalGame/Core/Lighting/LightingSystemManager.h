#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Subsystems/WorldSubsystem.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
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
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float SunPitch = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float SunYaw = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float AtmosphereIntensity = 1.0f;

    FLight_TimeSettings()
    {
        SunIntensity = 10.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
        SunPitch = 45.0f;
        SunYaw = 0.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
        AtmosphereIntensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FLinearColor WeatherTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float VisibilityDistance = 10000.0f;

    FLight_WeatherSettings()
    {
        CloudCoverage = 0.3f;
        RainIntensity = 0.0f;
        WindStrength = 1.0f;
        WeatherTint = FLinearColor::White;
        VisibilityDistance = 10000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting System")
    class USceneComponent* RootSceneComponent;

    // Time and weather settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationMinutes = 20.0f; // Real-time minutes for full day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bEnableWeatherSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherTransitionSpeed = 1.0f;

    // Lighting presets for different times
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    TMap<ELight_WeatherType, FLight_WeatherSettings> WeatherPresets;

    // References to world lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    class APostProcessVolume* PostProcessVolume;

public:
    // Day/Night cycle functions
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetDayNightCycleEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetDayDuration(float Minutes);

    // Weather system functions
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeather(ELight_WeatherType NewWeather, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherSystemEnabled(bool bEnabled);

    // Lighting control functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyTimeSettings(const FLight_TimeSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetupLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void CreateVolumetricLighting();

    // Preset management
    UFUNCTION(BlueprintCallable, Category = "Presets")
    void InitializeDefaultPresets();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void SaveCurrentAsPreset(ELight_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void LoadPreset(ELight_TimeOfDay TimeOfDay);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FLinearColor InterpolateColor(const FLinearColor& ColorA, const FLinearColor& ColorB, float Alpha) const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetNormalizedTimeOfDay() const; // Returns 0-1

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void LogLightingStatus() const;

private:
    // Internal update functions
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void UpdateSunPosition();
    void UpdateAtmosphere();
    void UpdateFog();
    void UpdatePostProcess();

    // Interpolation helpers
    FLight_TimeSettings GetInterpolatedTimeSettings() const;
    FLight_WeatherSettings GetInterpolatedWeatherSettings() const;

    // Weather transition state
    ELight_WeatherType TargetWeather;
    float WeatherTransitionProgress;
    bool bTransitioningWeather;
};