#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "CretaceousAtmosphereManager.generated.h"

/**
 * FLight_TimeOfDayConfig — Configuration for a specific time-of-day lighting preset.
 * Prefix: Light_ to avoid name collisions with other agents.
 */
USTRUCT(BlueprintType)
struct FLight_TimeOfDayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float SunPitch = -42.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float SunYaw = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float FogDensity = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    FLinearColor FogColor = FLinearColor(0.55f, 0.72f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float ExposureBias = 1.0f;
};

/**
 * ELight_TimeOfDay — Enumeration of time-of-day presets for the Cretaceous world.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"),
    Midday     UMETA(DisplayName = "Midday"),
    Afternoon  UMETA(DisplayName = "Afternoon"),
    GoldenHour UMETA(DisplayName = "Golden Hour"),
    Dusk       UMETA(DisplayName = "Dusk"),
    Night      UMETA(DisplayName = "Night")
};

/**
 * ACretaceousAtmosphereManager
 *
 * Manages the full Cretaceous lighting stack:
 * - Dynamic day/night cycle
 * - Lumen GI configuration
 * - SkyAtmosphere + SkyLight + ExponentialHeightFog
 * - PostProcess color grading
 * - Weather state transitions
 *
 * Place one instance in the level. It auto-discovers and configures
 * all lighting actors on BeginPlay.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Atmosphere Manager"))
class TRANSPERSONALGAME_API ACretaceousAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousAtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Time of Day ──────────────────────────────────────────────────────────

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TimeOfDayNormalized = 0.35f;

    /** Speed of day/night cycle (1.0 = real-time, 100.0 = fast) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayCycleSpeed = 0.0f;

    /** Whether the day/night cycle is running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bDayCycleActive = false;

    /** Current time of day preset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    ELight_TimeOfDay CurrentTimePreset = ELight_TimeOfDay::GoldenHour;

    // ── Lighting Presets ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig GoldenHourPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig MiddayPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig DawnPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig NightPreset;

    // ── Weather ───────────────────────────────────────────────────────────────

    /** Current weather fog density multiplier (1.0 = clear, 3.0 = heavy fog) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather",
        meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float WeatherFogMultiplier = 1.0f;

    /** Rain intensity (0.0 = dry, 1.0 = heavy rain) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RainIntensity = 0.0f;

    // ── References (auto-discovered) ─────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AActor> SunActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AActor> SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AActor> FogActor;

    // ── Blueprint-callable functions ─────────────────────────────────────────

    /** Apply a time-of-day preset immediately */
    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    void ApplyTimeOfDayPreset(ELight_TimeOfDay Preset);

    /** Apply a specific lighting config struct */
    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    void ApplyLightingConfig(const FLight_TimeOfDayConfig& Config);

    /** Set weather fog multiplier with smooth transition */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Weather")
    void SetWeatherFog(float FogMultiplier, float TransitionTime = 5.0f);

    /** Get current sun angle in degrees (0 = horizon, 90 = zenith) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|DayNight")
    float GetSunElevationDegrees() const;

    /** Returns true if it is currently daytime */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|DayNight")
    bool IsDaytime() const;

    /** Force apply the Cretaceous golden hour preset (editor utility) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Editor")
    void ForceApplyCretaceousGoldenHour();

private:
    void DiscoverLightingActors();
    void InitializeDefaultPresets();
    void TickDayCycle(float DeltaTime);

    float FogTransitionTarget = 0.03f;
    float FogTransitionSpeed = 0.0f;
    bool bFogTransitioning = false;
};
