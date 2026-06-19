#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycleManager.generated.h"

/**
 * Lighting presets for different times of day
 */
USTRUCT(BlueprintType)
struct FLight_TimeOfDayPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor = FLinearColor(0.1f, 0.12f, 0.18f, 1.0f);
};

/**
 * Time of day enum for the prehistoric survival game
 */
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

/**
 * ADayNightCycleManager
 * Manages the dynamic day/night cycle for the prehistoric world.
 * Controls sun position, fog, sky light, and atmospheric conditions.
 * Drives emotional tone through lighting — dawn = hope, midday = danger, dusk = tension, night = terror.
 */
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
    // ─── Time Settings ───────────────────────────────────────────────────────

    /** Current time of day in hours (0.0 = midnight, 12.0 = noon, 24.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    float CurrentTimeOfDay = 8.0f;

    /** Speed multiplier for time progression (1.0 = real-time, 60.0 = 1 min = 1 hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    float TimeScale = 60.0f;

    /** Whether the cycle is running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    bool bCycleActive = true;

    // ─── Light References ─────────────────────────────────────────────────────

    /** Reference to the main directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Lights")
    TObjectPtr<ADirectionalLight> SunLight;

    /** Reference to the sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Lights")
    TObjectPtr<ASkyLight> SkyLightActor;

    /** Reference to the exponential height fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Lights")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    // ─── Time of Day Presets ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDayPreset DawnPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDayPreset MiddayPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDayPreset DuskPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDayPreset NightPreset;

    // ─── Current State ────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle|State")
    ELight_TimeOfDay CurrentTimeEnum = ELight_TimeOfDay::Morning;

    // ─── Blueprint Events ─────────────────────────────────────────────────────

    /** Called when the time of day changes phase (dawn→morning, etc.) */
    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Cycle|Events")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay);

    /** Called every tick with normalized time (0..1) for Blueprint-driven effects */
    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Cycle|Events")
    void OnTimeUpdated(float NormalizedTime);

    // ─── Public API ───────────────────────────────────────────────────────────

    /** Set time of day directly (0..24) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    /** Get current time as 0..1 normalized */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    float GetNormalizedTime() const;

    /** Get current time of day enum */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    ELight_TimeOfDay GetTimeOfDayEnum() const;

    /** Force a specific preset immediately (for cutscenes) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyPreset(ELight_TimeOfDay TimeOfDay);

    /** Auto-find and assign light actors in the current level */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void AutoFindLightActors();

private:
    void UpdateLighting(float DeltaTime);
    void ApplyLightingPreset(const FLight_TimeOfDayPreset& Preset);
    FLight_TimeOfDayPreset InterpolatePresets(const FLight_TimeOfDayPreset& A, const FLight_TimeOfDayPreset& B, float Alpha) const;
    FLight_TimeOfDayPreset GetPresetForTime(float TimeHours) const;
    ELight_TimeOfDay TimeToEnum(float TimeHours) const;

    float PreviousTimeEnum = -1.0f;
};
