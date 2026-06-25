#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "CretaceousLightingSystem.generated.h"

/**
 * Dynamic day/night cycle and atmospheric lighting system for the Cretaceous world.
 * Controls sun position, sky color, fog density, and post-process settings
 * based on time of day. Integrates with Lumen GI for fully dynamic global illumination.
 */
UCLASS(ClassGroup = "Lighting", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACretaceousLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ─── Day/Night Cycle ───────────────────────────────────────────────────────

    /** Current time of day in hours (0.0 = midnight, 12.0 = noon, 24.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay = 8.5f;

    /** Speed multiplier for day/night cycle (1.0 = real time, 60.0 = 1 min per day) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float DayCycleSpeed = 60.0f;

    /** Whether the day/night cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    bool bDayCycleActive = true;

    // ─── Light References ──────────────────────────────────────────────────────

    /** Reference to the main directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> SunLight;

    /** Reference to the sky light for ambient GI */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    /** Reference to the atmospheric fog volume */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AExponentialHeightFog> AtmosphericFog;

    // ─── Sun Parameters ────────────────────────────────────────────────────────

    /** Sun intensity at noon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float NoonSunIntensity = 12.0f;

    /** Sun intensity at sunrise/sunset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float SunriseSunsetIntensity = 6.0f;

    /** Sun color at noon (warm daylight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor NoonSunColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);

    /** Sun color at sunrise/sunset (deep orange) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor SunriseColor = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);

    /** Sun color at night (moonlight blue) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor NightColor = FLinearColor(0.05f, 0.08f, 0.18f, 1.0f);

    // ─── Fog Parameters ────────────────────────────────────────────────────────

    /** Fog density during daytime */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DaytimeFogDensity = 0.02f;

    /** Fog density at night (denser for atmosphere) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NightFogDensity = 0.04f;

    /** Fog density at sunrise/sunset (golden hour haze) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float GoldenHourFogDensity = 0.035f;

    // ─── Weather State ─────────────────────────────────────────────────────────

    /** Current weather intensity (0.0 = clear, 1.0 = heavy storm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherIntensity = 0.0f;

    /** Whether a storm is currently active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    bool bStormActive = false;

    // ─── Blueprint Events ──────────────────────────────────────────────────────

    /** Called when time transitions to daytime (after sunrise) */
    UFUNCTION(BlueprintImplementableEvent, Category = "DayNight|Events")
    void OnDaytimeBegin();

    /** Called when time transitions to nighttime (after sunset) */
    UFUNCTION(BlueprintImplementableEvent, Category = "DayNight|Events")
    void OnNighttimeBegin();

    /** Called when golden hour begins (sunrise or sunset) */
    UFUNCTION(BlueprintImplementableEvent, Category = "DayNight|Events")
    void OnGoldenHourBegin(bool bIsSunrise);

    /** Set the time of day directly (0-24 hours) */
    UFUNCTION(BlueprintCallable, Category = "DayNight")
    void SetTimeOfDay(float NewTime);

    /** Get normalized time of day (0.0-1.0) */
    UFUNCTION(BlueprintPure, Category = "DayNight")
    float GetNormalizedTimeOfDay() const;

    /** Get whether it is currently daytime */
    UFUNCTION(BlueprintPure, Category = "DayNight")
    bool IsDaytime() const;

    /** Get whether it is currently golden hour (sunrise or sunset) */
    UFUNCTION(BlueprintPure, Category = "DayNight")
    bool IsGoldenHour() const;

    /** Trigger a weather transition */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Weather")
    void SetWeatherIntensity(float Intensity, float TransitionDuration = 30.0f);

private:
    /** Update sun position and color based on current time */
    void UpdateSunPosition(float DeltaTime);

    /** Update fog parameters based on time and weather */
    void UpdateFogParameters();

    /** Update sky light intensity based on time */
    void UpdateSkyLight();

    /** Interpolate between two linear colors */
    FLinearColor LerpSunColor(float Alpha) const;

    /** Calculate sun pitch angle from time of day */
    float CalculateSunPitch() const;

    /** Calculate sun yaw angle from time of day */
    float CalculateSunYaw() const;

    /** Whether we were in daytime last frame (for transition events) */
    bool bWasDaytime = true;

    /** Whether we were in golden hour last frame */
    bool bWasGoldenHour = false;

    /** Target weather intensity for smooth transitions */
    float TargetWeatherIntensity = 0.0f;

    /** Weather transition speed */
    float WeatherTransitionSpeed = 0.033f;
};
