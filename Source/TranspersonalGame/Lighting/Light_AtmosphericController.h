#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_AtmosphericController.generated.h"

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

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float BloomIntensity = 0.675f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float VignetteIntensity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float WhiteTemperature = 6500.0f;

    FLight_AtmosphericSettings()
    {
        // Default Cretaceous period settings
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericController : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Current atmospheric settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphericSettings CurrentSettings;

    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableDayNightCycle = true;

    // Weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsStormy = false;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class APostProcessVolume* PostProcessVolume;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TriggerStorm(float Duration = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ClearWeather();

protected:
    // Internal functions
    void UpdateSunPosition();
    void UpdateAtmosphericFog();
    void UpdatePostProcessing();
    void UpdateWeatherEffects();
    void FindAtmosphericComponents();
    
    FLight_AtmosphericSettings InterpolateSettings(const FLight_AtmosphericSettings& SettingsA, const FLight_AtmosphericSettings& SettingsB, float Alpha);

    // Preset settings for different times of day
    FLight_AtmosphericSettings DawnSettings;
    FLight_AtmosphericSettings NoonSettings;
    FLight_AtmosphericSettings DuskSettings;
    FLight_AtmosphericSettings NightSettings;

    // Storm timer
    float StormTimer = 0.0f;
    float StormDuration = 0.0f;
};