#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "LightingAtmosphereManager.generated.h"

// ============================================================
// Enums — global scope (UHT rule)
// ============================================================

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

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

// ============================================================
// Structs — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-45.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogInscatteringColor = FLinearColor(0.75f, 0.82f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureBias = 1.0f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityOverride = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionDuration = 30.0f;
};

// ============================================================
// ALightingAtmosphereManager — main actor
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Lighting & Atmosphere Manager"))
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ---- Time of Day ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float CurrentHour = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDurationSeconds = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bEnableDayNightCycle = true;

    UPROPERTY(BlueprintReadOnly, Category = "Time of Day")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    // ---- Weather ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLight_WeatherSettings CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherSystem = true;

    // ---- Light References ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* FogActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    APostProcessVolume* PostProcessVolume = nullptr;

    // ---- Preset Settings ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings NightSettings;

    // ---- Functions ----

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather, float TransitionTime = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentHour() const;

    UFUNCTION(BlueprintPure, Category = "Lighting")
    bool IsDaytime() const;

    UFUNCTION(BlueprintPure, Category = "Lighting")
    bool IsNighttime() const;

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ForceMiddayLighting();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ForceDawnLighting();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ForceDuskLighting();

private:
    void AdvanceTime(float DeltaTime);
    void UpdateSunPosition();
    void UpdateFog();
    void UpdateSkyLight();
    void UpdatePostProcess();
    ELight_TimeOfDay HourToTimeOfDay(float Hour) const;
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;

    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 30.0f;
    ELight_WeatherState TargetWeatherState = ELight_WeatherState::Clear;
    FLight_WeatherSettings PreviousWeather;
};
