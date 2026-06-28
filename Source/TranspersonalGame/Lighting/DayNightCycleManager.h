#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "DayNightCycleManager.generated.h"

// ============================================================
// Day/Night Cycle Manager — Agent #08 Lighting & Atmosphere
// Prehistoric survival game: realistic Cretaceous lighting
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
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Haze        UMETA(DisplayName = "Haze")
};

USTRUCT(BlueprintType)
struct FLight_SkyColorPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightningFrequency = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionDuration = 30.0f;
};

/**
 * ADayNightCycleManager
 * Controls the full day/night cycle for the prehistoric world.
 * Drives DirectionalLight pitch, sky color, fog density, and weather transitions.
 * Designed for Cretaceous era atmosphere — warm amber days, cool blue nights,
 * dramatic dawn/dusk transitions with volumetric god rays.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Time Control ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    float CurrentTimeOfDay = 8.0f; // 0-24 hours, starts at 8am

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    float DayDurationSeconds = 600.0f; // 10 minutes = 1 full day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    bool bPauseDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    float TimeScale = 1.0f;

    // ---- Light References ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Lights")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Lights")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Lights")
    AExponentialHeightFog* HeightFogActor = nullptr;

    // ---- Color Presets (Cretaceous Palette) ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Colors")
    FLight_SkyColorPreset DawnPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Colors")
    FLight_SkyColorPreset MorningPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Colors")
    FLight_SkyColorPreset MiddayPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Colors")
    FLight_SkyColorPreset DuskPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Colors")
    FLight_SkyColorPreset NightPreset;

    // ---- Weather ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Weather")
    FLight_WeatherConfig CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Weather")
    FLight_WeatherConfig TargetWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Weather")
    float WeatherTransitionAlpha = 0.0f;

    // ---- State ----

    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle|State", meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentTimeEnum = ELight_TimeOfDay::Morning;

    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle|State", meta = (AllowPrivateAccess = "true"))
    float CurrentSunPitch = -45.0f;

    // ---- Blueprint-callable functions ----

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewHour);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetWeather(ELight_WeatherState NewWeather, float TransitionTime = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentHour() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetTimeOfDayEnum() const { return CurrentTimeEnum; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    bool IsNight() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    bool IsDawn() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetNormalizedDayProgress() const;

    UFUNCTION(CallInEditor, Category = "Day Night Cycle")
    void AutoFindLightActors();

private:
    void AdvanceTime(float DeltaTime);
    void UpdateSunPosition();
    void UpdateSkyColors();
    void UpdateFog();
    void UpdateWeatherTransition(float DeltaTime);
    ELight_TimeOfDay HourToTimeEnum(float Hour) const;
    FLight_SkyColorPreset InterpolatePresets(const FLight_SkyColorPreset& A, const FLight_SkyColorPreset& B, float Alpha) const;
    FLight_SkyColorPreset GetPresetForHour(float Hour) const;
    float HourToSunPitch(float Hour) const;

    float WeatherTransitionTimer = 0.0f;
};
