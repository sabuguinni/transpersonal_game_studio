#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "DayNightCycleManager.generated.h"

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

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientIntensity;

    FLight_TimeOfDaySettings()
        : SunColor(FLinearColor(1.0f, 0.78f, 0.55f, 1.0f))
        , SunIntensity(8.5f)
        , SunPitchAngle(-45.0f)
        , FogColor(FLinearColor(0.78f, 0.82f, 0.86f, 1.0f))
        , FogDensity(0.02f)
        , AmbientIntensity(1.0f)
    {}
};

/**
 * ADayNightCycleManager
 * Manages the full 24-hour day/night cycle for the Cretaceous world.
 * Controls sun position, light color/intensity, fog density, and sky atmosphere.
 * Designed for Unreal Engine 5 Lumen global illumination.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ─── Time Settings ───────────────────────────────────────────────────────

    /** Current time of day in hours (0.0 = midnight, 12.0 = noon, 24.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay;

    /** Speed multiplier for the day/night cycle (1.0 = real time, 60.0 = 1 min per game hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float TimeScale;

    /** Whether the day/night cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive;

    // ─── Light References ─────────────────────────────────────────────────────

    /** Reference to the main directional light (sun/moon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ADirectionalLight* SunLight;

    // ─── Time of Day Presets ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings NightSettings;

    // ─── Current State ────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle", meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentTimeOfDayEnum;

    // ─── Blueprint Events ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Cycle")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Cycle")
    void OnSunrise();

    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Cycle")
    void OnSunset();

    // ─── Public API ───────────────────────────────────────────────────────────

    /** Set the time of day directly (0-24 hours) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    /** Get current time as ELight_TimeOfDay enum */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    /** Get normalized time (0.0 to 1.0) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetNormalizedTime() const;

    /** Apply lighting settings for current time */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyLightingForCurrentTime();

private:
    void UpdateSunPosition(float DeltaTime);
    void UpdateLightingColors();
    ELight_TimeOfDay ClassifyTimeOfDay(float TimeHours) const;
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;

    ELight_TimeOfDay PreviousTimeOfDayEnum;
    float TimeSinceLastSunriseCheck;
};
