#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "DayNightCycleManager.generated.h"

/**
 * Time-of-day states for the Cretaceous world.
 * Each state has distinct lighting, fog, and atmosphere parameters.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn (5-7am)"),
    Morning     UMETA(DisplayName = "Morning (7-11am)"),
    Noon        UMETA(DisplayName = "Noon (11am-2pm)"),
    GoldenHour  UMETA(DisplayName = "Golden Hour (4-6pm)"),
    Dusk        UMETA(DisplayName = "Dusk (6-8pm)"),
    Night       UMETA(DisplayName = "Night (8pm-5am)")
};

/**
 * Lighting preset for a specific time of day.
 * All values are interpolated smoothly between presets.
 */
USTRUCT(BlueprintType)
struct FLight_TimePreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::GoldenHour;

    /** Sun pitch angle — MUST be negative to illuminate terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "-90.0", ClampMax = "-5.0"))
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SkyLightIntensity = 1.5f;

    /** Normalized time (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NormalizedTime = 0.7f;
};

/**
 * ADayNightCycleManager
 *
 * Manages the full day/night cycle for the Cretaceous world.
 * Controls DirectionalLight (sun), SkyAtmosphere, ExponentialHeightFog, and SkyLight.
 * Interpolates smoothly between time-of-day presets.
 *
 * Place one instance in the level. Assign the lighting actors via editor.
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
    // =========================================================
    // REFERENCES — assign in editor or auto-found at BeginPlay
    // =========================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    // =========================================================
    // TIME SETTINGS
    // =========================================================

    /** Current time of day (0.0 = midnight, 0.5 = noon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentNormalizedTime = 0.7f;

    /** Real seconds per full in-game day */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    bool bPauseDayNightCycle = false;

    // =========================================================
    // TIME PRESETS
    // =========================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    TArray<FLight_TimePreset> TimePresets;

    // =========================================================
    // RUNTIME STATE
    // =========================================================

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    // =========================================================
    // BLUEPRINT-CALLABLE FUNCTIONS
    // =========================================================

    /** Set time of day immediately (no interpolation) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

    /** Get current time of day enum */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    /** Get current normalized time */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    float GetNormalizedTime() const { return CurrentNormalizedTime; }

    /** Force golden hour preset (default starting state) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyGoldenHourPreset();

    /** Force night preset */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyNightPreset();

private:
    void AdvanceTime(float DeltaTime);
    void ApplyLightingForTime(float NormalizedTime);
    FLight_TimePreset InterpolatePresets(const FLight_TimePreset& A, const FLight_TimePreset& B, float Alpha);
    ELight_TimeOfDay GetTimeOfDayEnum(float NormalizedTime) const;
    void AutoFindLightingActors();
    void InitializeDefaultPresets();
};
