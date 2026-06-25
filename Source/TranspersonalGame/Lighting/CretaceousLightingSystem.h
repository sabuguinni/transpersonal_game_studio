#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "CretaceousLightingSystem.generated.h"

/**
 * Lighting presets for different times of day in the Cretaceous world.
 * Used by ACretaceousLightingSystem to drive the day/night cycle.
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
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight"),
};

/**
 * Weather state that modulates lighting and fog parameters.
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
 * Data struct holding lighting parameters for a given time-of-day preset.
 */
USTRUCT(BlueprintType)
struct FLight_TimePreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    float SunPitch = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.62f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    float FogDensity = 0.025f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    FLinearColor FogColor = FLinearColor(0.85f, 0.72f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Preset")
    float ExposureBias = 0.5f;
};

/**
 * ACretaceousLightingSystem
 *
 * Master lighting controller for the prehistoric survival game.
 * Manages the full Lumen-based lighting stack:
 *   - Dynamic day/night cycle with smooth transitions
 *   - Weather-driven fog and atmosphere modulation
 *   - Directional sun light with Cretaceous warm palette
 *   - SkyLight real-time capture for accurate GI
 *   - Volumetric fog for jungle atmosphere
 *   - Post-process color grading for cinematic look
 *
 * Place one instance in the level. It auto-discovers lighting actors.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting System"))
class TRANSPERSONALGAME_API ACretaceousLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    // ── Day/Night Cycle ──────────────────────────────────────────────────

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TimeOfDayNormalized = 0.35f;  // Default: mid-morning

    /** Speed of the day/night cycle. 1.0 = real-time, 100.0 = 100× faster. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight",
        meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float DayCycleSpeed = 50.0f;

    /** Whether the day/night cycle is actively running. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bDayCycleActive = false;

    /** Current weather state affecting fog and sky. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    // ── Lighting References ──────────────────────────────────────────────

    /** Reference to the scene's directional light (sun). Auto-found if null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> SunLight;

    /** Reference to the scene's sky light. Auto-found if null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ASkyLight> SceneSkyLight;

    /** Reference to the scene's height fog. Auto-found if null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AExponentialHeightFog> SceneFog;

    /** Reference to the scene's post process volume. Auto-found if null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<APostProcessVolume> ScenePostProcess;

    // ── Preset Overrides ─────────────────────────────────────────────────

    /** Golden hour preset — default Cretaceous look. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimePreset GoldenHourPreset;

    /** Midday preset — harsh overhead sun. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimePreset MiddayPreset;

    /** Night preset — moonlit darkness. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimePreset NightPreset;

    // ── Blueprint Functions ───────────────────────────────────────────────

    /** Apply a specific time-of-day preset immediately. */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void ApplyTimePreset(ELight_TimeOfDay TimeOfDay);

    /** Set weather and update fog/atmosphere parameters. */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void SetWeather(ELight_WeatherState NewWeather);

    /** Force-apply the current TimeOfDayNormalized to all lighting actors. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Control")
    void RefreshLighting();

    /** Auto-discover all lighting actors in the current level. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Control")
    void AutoDiscoverLightingActors();

    /** Get the current sun pitch angle based on TimeOfDayNormalized. */
    UFUNCTION(BlueprintPure, Category = "Lighting|Query")
    float GetCurrentSunPitch() const;

    /** Get the current ambient light color for the time of day. */
    UFUNCTION(BlueprintPure, Category = "Lighting|Query")
    FLinearColor GetCurrentSkyColor() const;

private:
    void TickDayCycle(float DeltaTime);
    void ApplyPresetToScene(const FLight_TimePreset& Preset);
    FLight_TimePreset InterpolatePresets(const FLight_TimePreset& A, const FLight_TimePreset& B, float Alpha);
    FLight_TimePreset GetPresetForTime(float NormalizedTime) const;

    float PreviousTimeOfDay = -1.0f;
};
