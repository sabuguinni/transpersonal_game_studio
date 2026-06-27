#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "CretaceousLightingManager.generated.h"

/**
 * Lighting preset for different times of day in the Cretaceous world.
 * Controls sun angle, colour temperature, fog density, and Lumen settings.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Storm       UMETA(DisplayName = "Storm"),
};

/**
 * Lighting preset data — defines the visual parameters for each time of day.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_DayPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::GoldenHour;

    /** Sun pitch angle in degrees. Must be <= -30 (CAP enforcement). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMax = "-30.0"))
    float SunPitch = -35.0f;

    /** Sun yaw angle in degrees. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    /** Sun intensity in lux. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0"))
    float SunIntensity = 8.5f;

    /** Sun colour (linear). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    /** Sky light intensity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0"))
    float SkyIntensity = 1.8f;

    /** Exponential height fog density. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FogDensity = 0.035f;

    /** Fog inscattering colour (linear). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);

    /** Whether volumetric fog is enabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;

    /** Volumetric fog scattering distribution (0=isotropic, 1=forward). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VolumetricFogScattering = 0.35f;

    /** Transition blend time in seconds when switching presets. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0"))
    float BlendDuration = 5.0f;
};

/**
 * ACretaceousLightingManager
 *
 * Manages the dynamic day/night cycle and weather lighting for the Cretaceous world.
 * Drives DirectionalLight, SkyLight, ExponentialHeightFog, and Lumen settings
 * in real time based on the current time-of-day preset.
 *
 * Placed once in the level. Configured via Blueprint or C++ presets.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting Manager"))
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Preset Library ────────────────────────────────────────────────────────

    /** All available time-of-day presets. Populated with defaults in constructor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    TArray<FLight_DayPreset> DayPresets;

    /** Currently active preset index. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    int32 ActivePresetIndex = 3; // GoldenHour by default

    // ── Scene References ──────────────────────────────────────────────────────

    /** Reference to the level's DirectionalLight (sun). Auto-found on BeginPlay if null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> SunLight;

    /** Reference to the level's SkyLight. Auto-found on BeginPlay if null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ASkyLight> SceneSkyLight;

    /** Reference to the level's ExponentialHeightFog. Auto-found on BeginPlay if null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AExponentialHeightFog> SceneFog;

    // ── Day/Night Cycle ───────────────────────────────────────────────────────

    /** Whether the day/night cycle advances automatically. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bAutoCycle = false;

    /** Real-world seconds per full in-game day. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight", meta = (ClampMin = "60.0"))
    float SecondsPerDay = 1200.0f;

    /** Current normalised time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|DayNight")
    float NormalisedTimeOfDay = 0.35f; // Start at golden hour

    // ── Blend State ───────────────────────────────────────────────────────────

    /** Whether a preset blend is currently in progress. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|Blend")
    bool bBlending = false;

    /** Blend progress (0-1). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|Blend")
    float BlendAlpha = 0.0f;

    // ── Blueprint API ─────────────────────────────────────────────────────────

    /** Immediately apply a preset by index. */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyPreset(int32 PresetIndex);

    /** Begin blending to a preset over its BlendDuration. */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void BlendToPreset(int32 PresetIndex);

    /** Apply the golden-hour Cretaceous default (pitch=-35, warm amber). */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyGoldenHourDefault();

    /** Enforce CAP rules: sun pitch <= -30, fog count = 1. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void EnforceCAPRules();

    /** Return the display name of the current preset. */
    UFUNCTION(BlueprintPure, Category = "Lighting")
    FString GetCurrentPresetName() const;

private:
    /** Source preset when blending. */
    FLight_DayPreset BlendSource;

    /** Target preset when blending. */
    FLight_DayPreset BlendTarget;

    /** Elapsed blend time in seconds. */
    float BlendElapsed = 0.0f;

    /** Apply a fully-resolved preset struct to the scene lights. */
    void ApplyPresetToScene(const FLight_DayPreset& Preset);

    /** Interpolate between two presets by alpha [0-1]. */
    FLight_DayPreset LerpPresets(const FLight_DayPreset& A, const FLight_DayPreset& B, float Alpha) const;

    /** Auto-find scene light actors if references are null. */
    void AutoFindSceneLights();

    /** Build the default preset library. */
    void BuildDefaultPresets();
};
