#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycleManager.generated.h"

// ============================================================
// Enums — Light_* prefix to avoid collision with other agents
// ============================================================

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Twilight    UMETA(DisplayName = "Twilight"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Hazy        UMETA(DisplayName = "Hazy")
};

// ============================================================
// Structs — Light_* prefix
// ============================================================

USTRUCT(BlueprintType)
struct FLight_SkyPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientOcclusionIntensity = 0.5f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherParams
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
    float WindSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TemperatureModifier = 0.0f;
};

// ============================================================
// ADayNightCycleManager — Main Actor
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ---- Time Control ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDayHours = 12.0f;  // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationSeconds = 600.0f;  // 10 min real = 1 game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeMultiplier = 1.0f;

    // ---- Scene References ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ASkyLight* SkyLightActor = nullptr;

    // ---- Weather ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLight_WeatherParams CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.5f;

    // ---- Palette Presets ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette MorningPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette NightPalette;

    // ---- State (read-only) ----

    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentTimeOfDayEnum = ELight_TimeOfDay::Midday;

    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float CurrentSunPitch = -45.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float NormalizedDayProgress = 0.5f;  // 0=midnight, 0.5=noon, 1=midnight

    // ---- Blueprint Events ----

    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Cycle")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather")
    void OnWeatherChanged(ELight_WeatherState NewWeather);

    // ---- Public API ----

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float HoursIn24);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetTimeOfDayHours() const { return CurrentTimeOfDayHours; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const { return CurrentTimeOfDayEnum; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    bool IsNight() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    bool IsDawn() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(ELight_WeatherState NewWeather, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    FLight_WeatherParams GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugApplyCurrentPalette();

private:
    void AdvanceTime(float DeltaTime);
    void UpdateSunPosition();
    void UpdateAtmosphere();
    void UpdateSkyLight();
    ELight_TimeOfDay HoursToTimeOfDay(float Hours) const;
    FLight_SkyPalette InterpolatePalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const;
    FLight_SkyPalette GetPaletteForTime(float Hours) const;
    void ApplyPaletteToScene(const FLight_SkyPalette& Palette);

    ELight_TimeOfDay PreviousTimeOfDayEnum = ELight_TimeOfDay::Midday;
    float WeatherTransitionTimer = 0.0f;
    ELight_WeatherState TargetWeather = ELight_WeatherState::Clear;
};
