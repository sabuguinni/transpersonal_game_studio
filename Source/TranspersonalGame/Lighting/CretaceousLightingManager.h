#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "CretaceousLightingManager.generated.h"

/**
 * Manages the full Cretaceous lighting stack:
 * - Dynamic day/night cycle with sun arc
 * - Lumen GI + volumetric fog
 * - Atmospheric scattering (SkyAtmosphere)
 * - Weather state transitions (clear, overcast, storm)
 * - Golden hour / blue hour color grading
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Hazy        UMETA(DisplayName = "Hazy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Storm       UMETA(DisplayName = "Storm"),
    Rain        UMETA(DisplayName = "Rain")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDayConfig
{
    GENERATED_BODY()

    /** Sun pitch angle in degrees (negative = above horizon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -38.0f;

    /** Sun yaw angle in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    /** Sun intensity in lux */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    /** Sun color (warm golden for golden hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.62f, 1.0f);

    /** Fog density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.018f;

    /** Fog inscattering color (Cretaceous blue-green haze) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.45f, 0.62f, 0.85f, 1.0f);

    /** SkyLight intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    /** Post process exposure bias */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureBias = 0.3f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ─── Day/Night Cycle ───────────────────────────────────────────────────────

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float TimeOfDayNormalized = 0.583f; // Default: ~14:00 (golden hour)

    /** Real-time seconds per in-game day (0 = frozen) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationSeconds = 0.0f;

    /** Whether the day/night cycle is actively ticking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bDayCycleActive = false;

    /** Current time of day enum state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    // ─── Weather ───────────────────────────────────────────────────────────────

    /** Current weather state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    /** Weather transition blend speed (0-1 per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.1f;

    // ─── Scene References ──────────────────────────────────────────────────────

    /** Reference to the scene's directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ADirectionalLight* SunLight = nullptr;

    /** Reference to the scene's sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ASkyLight* SkyLightActor = nullptr;

    // ─── Config ────────────────────────────────────────────────────────────────

    /** Golden hour lighting configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Config")
    FLight_TimeOfDayConfig GoldenHourConfig;

    /** Midday lighting configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Config")
    FLight_TimeOfDayConfig MiddayConfig;

    /** Dawn lighting configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Config")
    FLight_TimeOfDayConfig DawnConfig;

    /** Night lighting configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Config")
    FLight_TimeOfDayConfig NightConfig;

    // ─── Blueprint Functions ───────────────────────────────────────────────────

    /** Set time of day directly (0.0-1.0) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

    /** Transition to a specific weather state */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    /** Apply the current lighting config to all scene components */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingConfig(const FLight_TimeOfDayConfig& Config);

    /** Get the current time of day as a human-readable string */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    FString GetTimeOfDayString() const;

    /** Auto-find and cache scene lighting actors */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void AutoFindLightingActors();

private:
    /** Compute sun rotation from normalized time */
    FRotator ComputeSunRotation(float NormalizedTime) const;

    /** Interpolate between two lighting configs */
    FLight_TimeOfDayConfig LerpLightingConfig(const FLight_TimeOfDayConfig& A, const FLight_TimeOfDayConfig& B, float Alpha) const;

    /** Determine time of day enum from normalized time */
    ELight_TimeOfDay ClassifyTimeOfDay(float NormalizedTime) const;

    float WeatherBlendAlpha = 0.0f;
};
