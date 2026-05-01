#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Light_AtmosphereManager.generated.h"

// Time of day states for prehistoric lighting
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

// Weather states for atmospheric effects
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Misty       UMETA(DisplayName = "Misty")
};

// Lighting settings for different times of day
USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
    GENERATED_BODY()

    // Sun angle and intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    // Sky and atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    // Fog settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    FLight_TimeSettings()
    {
        SunAngle = 45.0f;
        SunIntensity = 3.0f;
        SunColor = FLinearColor::White;
        SkyLightIntensity = 1.0f;
        SkyColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        FogHeightFalloff = 0.2f;
    }
};

/**
 * Manages dynamic lighting and atmospheric effects for prehistoric environments
 * Handles day/night cycle, weather transitions, and Lumen global illumination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    class USceneComponent* RootSceneComponent;

    // References to lighting actors in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class AExponentialHeightFog* HeightFog;

    // Day/night cycle settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimeState = ELight_TimeOfDay::Midday;

    // Weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.1", ClampMax = "60.0"))
    float WeatherTransitionTime = 5.0f;

    // Lighting presets for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings MorningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings AfternoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings NightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeSettings MidnightSettings;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLumenGlobalIllumination = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLumenReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float LightingUpdateFrequency = 1.0f;

public:
    // Public interface functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void TransitionToTimeOfDay(ELight_TimeOfDay TargetTime, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyLightingPreset(const FLight_TimeSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void FindAndAssignLightingActors();

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    ELight_TimeOfDay GetCurrentTimeState() const { return CurrentTimeState; }

    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    ELight_WeatherState GetCurrentWeather() const { return CurrentWeather; }

protected:
    // Internal update functions
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherEffects(float DeltaTime);
    void UpdateSunPosition();
    void UpdateSkyLighting();
    void UpdateFogEffects();
    ELight_TimeOfDay CalculateTimeState(float TimeOfDay);
    FLight_TimeSettings GetSettingsForTime(ELight_TimeOfDay TimeState);
    FLight_TimeSettings InterpolateSettings(const FLight_TimeSettings& A, const FLight_TimeSettings& B, float Alpha);

private:
    // Internal state
    float LastUpdateTime = 0.0f;
    bool bIsTransitioning = false;
    float TransitionStartTime = 0.0f;
    float TransitionDuration = 0.0f;
    FLight_TimeSettings TransitionStartSettings;
    FLight_TimeSettings TransitionTargetSettings;
};