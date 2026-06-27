#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "CretaceousLightingManager.generated.h"

/**
 * Lighting phase enum for Cretaceous day/night cycle.
 * Prefix: Light_ to avoid name collisions.
 */
UENUM(BlueprintType)
enum class ELight_DayPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    DeepNight   UMETA(DisplayName = "Deep Night"),
};

/**
 * Weather state enum for atmospheric variation.
 */
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Hazy        UMETA(DisplayName = "Hazy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
};

/**
 * Lighting preset data for a specific time of day.
 */
USTRUCT(BlueprintType)
struct FLight_DayPhasePreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_DayPhase Phase = ELight_DayPhase::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.85f, 0.72f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogHeightFalloff = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricFogExtinctionScale = 1.2f;
};

/**
 * ACretaceousLightingManager
 *
 * Manages the full Cretaceous day/night cycle, weather transitions,
 * and atmospheric lighting using Lumen + SkyAtmosphere + ExponentialHeightFog.
 * Placed once in the level — drives all dynamic lighting.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (DisplayName = "Cretaceous Lighting Manager"))
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ─── Day/Night Cycle ───────────────────────────────────────────────────

    /** Current time of day in hours (0.0 = midnight, 12.0 = noon, 24.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay = 7.5f;

    /** Speed multiplier for day/night cycle (1.0 = real-time, 60.0 = 1 min per hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float DayCycleSpeed = 60.0f;

    /** Whether the day/night cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bDayCycleActive = true;

    /** Current day phase (read-only, computed from TimeOfDay) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_DayPhase CurrentDayPhase = ELight_DayPhase::GoldenHour;

    // ─── Weather ───────────────────────────────────────────────────────────

    /** Current weather state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    /** Blend speed for weather transitions (0.0-1.0 per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.001", ClampMax = "1.0"))
    float WeatherBlendSpeed = 0.05f;

    // ─── References ────────────────────────────────────────────────────────

    /** Reference to the scene's directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight = nullptr;

    /** Reference to the scene's sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ASkyLight* SceneSkyLight = nullptr;

    /** Reference to the scene's exponential height fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* SceneFog = nullptr;

    // ─── Presets ───────────────────────────────────────────────────────────

    /** Day phase lighting presets — one per phase */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TArray<FLight_DayPhasePreset> DayPhasePresets;

    // ─── Blueprint Events ──────────────────────────────────────────────────

    /** Called when the day phase changes */
    UFUNCTION(BlueprintImplementableEvent, Category = "Day/Night Cycle")
    void OnDayPhaseChanged(ELight_DayPhase NewPhase, ELight_DayPhase OldPhase);

    /** Called when weather changes */
    UFUNCTION(BlueprintImplementableEvent, Category = "Weather")
    void OnWeatherChanged(ELight_WeatherState NewWeather);

    /** Force a specific time of day immediately */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NewTime);

    /** Force a specific weather state */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(ELight_WeatherState NewWeather);

    /** Apply the golden hour preset (Cretaceous afternoon sun) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Presets")
    void ApplyGoldenHourPreset();

    /** Apply the midday preset (harsh overhead sun) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Presets")
    void ApplyMiddayPreset();

    /** Apply the night preset (moonlight + bioluminescence) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Presets")
    void ApplyNightPreset();

    /** Auto-discover sun, sky, fog from the level */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void AutoDiscoverLightingActors();

private:
    void AdvanceDayCycle(float DeltaTime);
    void UpdateLightingForTime(float Time);
    ELight_DayPhase ComputeDayPhase(float Time) const;
    void ApplyPreset(const FLight_DayPhasePreset& Preset);
    FLight_DayPhasePreset GetDefaultPreset(ELight_DayPhase Phase) const;
    void InitializeDefaultPresets();

    ELight_DayPhase PreviousDayPhase = ELight_DayPhase::GoldenHour;
    float CurrentBlendAlpha = 1.0f;
    FLight_DayPhasePreset CurrentPreset;
    FLight_DayPhasePreset TargetPreset;
};
