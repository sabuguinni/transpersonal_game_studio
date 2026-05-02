#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Light_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Storm       UMETA(DisplayName = "Storm"),
    Rain        UMETA(DisplayName = "Rain"),
    Fog         UMETA(DisplayName = "Heavy Fog")
};

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
struct FLight_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Weather and time control
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetDayNightCycle(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TriggerLightning();

    // Lighting configuration
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingSettings(const FLight_LightingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition(float TimeOfDayNormalized);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateFogSettings(ELight_WeatherState Weather);

protected:
    // Core lighting actors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<ASkyLight> SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<AExponentialHeightFog> AtmosphereFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<AExponentialHeightFog> StormFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<APointLight> LightningLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<ASkyLight> RainSkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<APostProcessVolume> AtmospherePostProcess;

    // Atmospheric lights
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    TArray<TObjectPtr<ASpotLight>> ForestSpotLights;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    TArray<TObjectPtr<APointLight>> AmbientPointLights;

    // Current state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Noon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bDayNightCycleEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float DayDuration = 1200.0f; // 20 minutes real time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float CurrentDayTime = 0.5f; // 0.0 = midnight, 0.5 = noon

    // Lightning system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightningFlashDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightningCooldown = 5.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather")
    float LastLightningTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather")
    bool bLightningActive = false;

    // Lighting presets for different times/weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_TimeOfDay, FLight_LightingSettings> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_WeatherState, FLight_LightingSettings> WeatherPresets;

private:
    void FindLightingActors();
    void InitializePresets();
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateLightning(float DeltaTime);
    FLight_LightingSettings GetCurrentLightingSettings() const;
    void InterpolateLightingSettings(const FLight_LightingSettings& From, const FLight_LightingSettings& To, float Alpha);
};