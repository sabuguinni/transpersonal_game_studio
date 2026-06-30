#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "DayNightCycleManager.generated.h"

// ============================================================
// ELight_TimeOfDay — Cretaceous day/night cycle phases
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

// ============================================================
// ELight_WeatherState — Atmospheric weather conditions
// ============================================================
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain")
};

// ============================================================
// FLight_SkyPalette — Complete sky/lighting configuration for a time of day
// ============================================================
USTRUCT(BlueprintType)
struct FLight_SkyPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogInscatteringColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricFogExtinction = 1.0f;
};

// ============================================================
// FLight_WeatherModifier — Weather overlay on top of sky palette
// ============================================================
USTRUCT(BlueprintType)
struct FLight_WeatherModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SunIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogTintOverride = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableVolumetricClouds = false;
};

// ============================================================
// ADayNightCycleManager — Master lighting controller actor
// Manages Cretaceous day/night cycle, weather, and atmosphere
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Current State ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Dawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    /** Current time in hours (0.0 = midnight, 6.0 = dawn, 12.0 = midday, 18.0 = dusk) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 6.0f;

    /** How many real seconds equal one in-game hour */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "1.0"))
    float SecondsPerGameHour = 120.0f;

    /** Enable automatic time progression */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bAutoAdvanceTime = true;

    // ---- Scene References ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    AActor* FogActor = nullptr;

    // ---- Palette Configuration ----
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

    // ---- Weather Modifiers ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLight_WeatherModifier StormModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLight_WeatherModifier FoggyModifier;

    // ---- Blueprint-callable Functions ----

    /** Force a specific time of day immediately */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    /** Force a specific hour (0-24) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetHour(float Hour);

    /** Transition to a new weather state */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetWeather(ELight_WeatherState NewWeather);

    /** Get the current palette for the active time of day */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FLight_SkyPalette GetCurrentPalette() const;

    /** Get sun pitch angle for current hour */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetSunPitchForHour(float Hour) const;

    /** Apply a palette to scene lighting immediately */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyPalette(const FLight_SkyPalette& Palette);

    /** Auto-detect scene actors (sun, sky, fog) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void AutoDetectSceneActors();

    /** Preview current time of day in editor */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void PreviewCurrentTime();

private:
    float TimeAccumulator = 0.0f;

    ELight_TimeOfDay HourToTimeOfDay(float Hour) const;
    void AdvanceTime(float DeltaTime);
    void UpdateLighting();
    FLight_SkyPalette LerpPalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const;
    void InitializeDefaultPalettes();
};
