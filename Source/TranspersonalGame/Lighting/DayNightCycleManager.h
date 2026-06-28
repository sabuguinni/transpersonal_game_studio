#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "DayNightCycleManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

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
    Overcast    UMETA(DisplayName = "Overcast"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Hazy        UMETA(DisplayName = "Hazy")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    /** Normalized time of day [0.0 = midnight, 0.5 = noon, 1.0 = midnight] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float NormalizedTime = 0.35f;

    /** Sun pitch angle in degrees (negative = above horizon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SunPitchDegrees = -45.0f;

    /** Sun yaw angle in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SunYawDegrees = 60.0f;

    /** Sun light color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    /** Sun intensity in lux */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SunIntensityLux = 120000.0f;

    /** Ambient sky color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    /** Fog density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float FogDensity = 0.02f;

    /** Fog inscattering color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float FogMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float LightIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float TransitionDurationSeconds = 30.0f;
};

// ─── Main Class ───────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Configuration ────────────────────────────────────────────────────────

    /** Full day duration in real-world seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayDurationSeconds = 600.0f;

    /** Whether the cycle is currently running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bCycleRunning = true;

    /** Starting time of day [0.0–1.0] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float StartingNormalizedTime = 0.35f;

    /** Current weather state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    FLight_WeatherSettings CurrentWeather;

    // ── References (auto-found or manually assigned) ──────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    // ── Runtime State ─────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|DayNight")
    float CurrentNormalizedTime = 0.35f;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|DayNight")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Morning;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|DayNight")
    int32 CurrentDay = 1;

    // ── Blueprint Events ──────────────────────────────────────────────────

    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|DayNight")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay, float NormalizedTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|DayNight")
    void OnNewDayStarted(int32 DayNumber);

    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|DayNight")
    void OnWeatherChanged(ELight_WeatherState NewWeather);

    // ── Public API ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    void SetNormalizedTime(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    void SetWeather(ELight_WeatherState NewWeather, float TransitionDuration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    float GetNormalizedTime() const { return CurrentNormalizedTime; }

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    ELight_TimeOfDay GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    float GetSunIntensityMultiplier() const;

    UFUNCTION(BlueprintPure, Category = "Lighting|DayNight")
    FString GetTimeOfDayString() const;

protected:
    // ── Internal helpers ──────────────────────────────────────────────────

    void FindLightActors();
    void UpdateSunPosition(float NormalizedTime);
    void UpdateFogSettings(float NormalizedTime);
    void UpdateSkyLight();
    void ApplyWeatherMultipliers();
    ELight_TimeOfDay ClassifyTimeOfDay(float NormalizedTime) const;
    FLight_TimeOfDaySettings InterpolateTimeSettings(float NormalizedTime) const;
    FLinearColor LerpColor(const FLinearColor& A, const FLinearColor& B, float Alpha) const;

    // ── Keyframe presets (Dawn/Morning/Midday/Afternoon/Dusk/Night) ───────
    static const TArray<FLight_TimeOfDaySettings>& GetTimeKeyframes();

    // ── Weather transition ────────────────────────────────────────────────
    float WeatherTransitionAlpha = 1.0f;
    float WeatherTransitionDuration = 30.0f;
    ELight_WeatherState PreviousWeather = ELight_WeatherState::Clear;
    ELight_WeatherState TargetWeather = ELight_WeatherState::Clear;
    bool bWeatherTransitioning = false;

    ELight_TimeOfDay PreviousTimeOfDay = ELight_TimeOfDay::Morning;
};
