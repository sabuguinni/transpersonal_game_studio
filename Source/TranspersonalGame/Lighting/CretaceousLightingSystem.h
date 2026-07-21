#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "CretaceousLightingSystem.generated.h"

// ============================================================
// Cretaceous Lighting System — Agent #08 Lighting & Atmosphere
// Manages dynamic day/night cycle with Lumen GI for prehistoric
// survival game. Warm amber days, cool blue nights, storm system.
// ============================================================

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyStorm  UMETA(DisplayName = "Heavy Storm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchAngle = -35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawAngle = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.784f, 0.549f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Day/Night Cycle ──────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayDurationSeconds = 600.0f;  // 10 real minutes = 1 game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float CurrentTimeOfDayNormalized = 0.35f;  // 0=midnight, 0.5=noon, 1=midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bDayNightCycleEnabled = true;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|DayNight",
        meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    // ── Weather ──────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float WeatherTransitionSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float StormProbabilityPerHour = 0.15f;

    // ── Scene References ─────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* FogActor = nullptr;

    // ── Time of Day Presets ──────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDaySettings GoldenHourSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDaySettings NightSettings;

    // ── Blueprint Callable Functions ─────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetSunriseTime() const { return 0.25f; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetSunsetTime() const { return 0.75f; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyNightPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyStormPreset();

private:
    void UpdateSunPosition(float NormalizedTime);
    void UpdateSkyAndFog(float NormalizedTime);
    void UpdateTimeOfDayEnum(float NormalizedTime);
    FLight_TimeOfDaySettings InterpolateSettings(
        const FLight_TimeOfDaySettings& A,
        const FLight_TimeOfDaySettings& B,
        float Alpha) const;

    float ElapsedDayTime = 0.0f;
    float WeatherTransitionAlpha = 0.0f;
    ELight_WeatherState TargetWeather = ELight_WeatherState::Clear;
};
