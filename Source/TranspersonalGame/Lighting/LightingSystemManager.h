#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Sunset      UMETA(DisplayName = "Sunset"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunLightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float SunLightIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float FogHeightFalloff = 0.2f;

    FLight_AtmosphereSettings()
    {
        SunLightColor = FLinearColor::White;
        SunLightIntensity = 3.0f;
        SunRotation = FRotator(-45.0f, 0.0f, 0.0f);
        SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
        SkyLightIntensity = 1.0f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ASkyLight* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class AExponentialHeightFog* HeightFog;

    // Time and weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TimeSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherTransitionProgress = 0.0f;

    // Atmosphere presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereSettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereSettings SunsetSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereSettings NightSettings;

    // Lumen settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bUseLumenGlobalIllumination = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bUseLumenReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen", meta = (ClampMin = "0.1", ClampMax = "4.0"))
    float LumenGIIntensity = 1.0f;

public:
    // Time of day functions
    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeSpeed(float NewSpeed);

    // Weather functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(ELight_WeatherState NewWeatherState, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToRandomWeather();

    // Lighting control functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLightingForTimeOfDay();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_AtmosphereSettings GetCurrentAtmosphereSettings() const;

    // Lumen control
    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void SetLumenEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void SetLumenIntensity(float Intensity);

    // Interior lighting
    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void SetInteriorLightingMode(bool bIsInterior);

private:
    void InitializeLightingComponents();
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    FLight_AtmosphereSettings InterpolateAtmosphereSettings(const FLight_AtmosphereSettings& A, const FLight_AtmosphereSettings& B, float Alpha) const;
    void ApplyLumenSettings();

    // Weather transition
    ELight_WeatherState TargetWeatherState = ELight_WeatherState::Clear;
    float WeatherTransitionDuration = 5.0f;
    float WeatherTransitionTimer = 0.0f;
    bool bIsTransitioningWeather = false;

    // Interior mode
    bool bIsInteriorMode = false;
    FLight_AtmosphereSettings CachedExteriorSettings;
};