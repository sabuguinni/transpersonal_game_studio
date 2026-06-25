#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Atmosphere/AtmosphericFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — must be at global scope (UHT rule)
// ─────────────────────────────────────────────────────────────────────────────

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
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs — must be at global scope (UHT rule)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FLight_TimeOfDayPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawDegrees = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.619f, 1.0f); // Warm gold

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientOcclusionIntensity = 0.6f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionDurationSeconds = 30.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// ACretaceousLightingSystem — Main lighting controller actor
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting System"))
class TRANSPERSONALGAME_API ACretaceousLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingSystem();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    // ── Time of Day ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 14.5f; // 2:30 PM golden hour default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    TArray<FLight_TimeOfDayPreset> TimeOfDayPresets;

    // ── Weather ────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    TArray<FLight_WeatherPreset> WeatherPresets;

    // ── Component References ───────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDirectionalLightComponent> SunComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USkyLightComponent> SkyLightComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UExponentialHeightFogComponent> FogComponent;

    // ── Lumen Settings ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bEnableLumen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    float LumenMaxTraceDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bEnableVolumetricFog = true;

    // ── Blueprint Callable Functions ───────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather, float TransitionDuration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetCurrentHour(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentHour() const { return CurrentHour; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyNightPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyStormPreset();

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyCurrentSettingsInEditor();

private:
    // ── Internal State ─────────────────────────────────────────────────────
    float WeatherTransitionElapsed = 0.0f;
    float WeatherTransitionDuration = 30.0f;
    ELight_WeatherState TargetWeather = ELight_WeatherState::Clear;
    bool bTransitioningWeather = false;

    // ── Internal Helpers ───────────────────────────────────────────────────
    void TickDayNightCycle(float DeltaTime);
    void TickWeatherTransition(float DeltaTime);
    void UpdateSunRotation();
    void UpdateLightingFromHour(float Hour);
    ELight_TimeOfDay HourToTimeOfDay(float Hour) const;
    FLight_TimeOfDayPreset GetPresetForTimeOfDay(ELight_TimeOfDay TOD) const;
    FLight_TimeOfDayPreset GetDefaultGoldenHourPreset() const;
    void ApplyPreset(const FLight_TimeOfDayPreset& Preset);
    void FindOrCreateLightingActors();
};
