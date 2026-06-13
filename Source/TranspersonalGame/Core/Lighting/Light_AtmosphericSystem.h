#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Light_AtmosphericSystem.generated.h"

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
    Hazy        UMETA(DisplayName = "Hazy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.71f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 5800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogInscatteringColor = FLinearColor(0.71f, 0.63f, 0.47f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float WhiteTemperature = 5600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float ColorSaturation = 1.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float ColorContrast = 1.1f;

    FLight_AtmosphericSettings()
    {
        // Default Cretaceous period settings
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TSoftObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TSoftObjectPtr<ASkyLight> SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TSoftObjectPtr<AExponentialHeightFog> AtmosphericFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TSoftObjectPtr<APostProcessVolume> PostProcessVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float DayNightCycleDuration = 1200.0f; // 20 minutes real time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bEnableWeatherTransitions = true;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentTimeOfDayFloat = 14.0f; // 2 PM

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float WeatherTransitionProgress = 0.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(ELight_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StartDayNightCycle();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopDayNightCycle();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDuration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void FindAndLinkLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void CreateCretaceousLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void EnableLumenGlobalIllumination();

    UFUNCTION(BlueprintPure, Category = "Time")
    float GetNormalizedTimeOfDay() const;

    UFUNCTION(BlueprintPure, Category = "Sun")
    FRotator CalculateSunRotation(float TimeOfDay) const;

    UFUNCTION(BlueprintPure, Category = "Settings")
    FLight_AtmosphericSettings GetSettingsForTimeAndWeather(ELight_TimeOfDay TimeOfDay, ELight_WeatherType Weather) const;

protected:
    void UpdateLightingForTimeOfDay();
    void UpdateWeatherEffects();
    void InterpolateLightingSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha);

private:
    float DayNightCycleTimer = 0.0f;
    ELight_WeatherType TargetWeatherType = ELight_WeatherType::Clear;
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 30.0f;
    bool bIsTransitioningWeather = false;
};