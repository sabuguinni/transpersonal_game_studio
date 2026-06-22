#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycleManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"),
    Noon       UMETA(DisplayName = "Noon"),
    Afternoon  UMETA(DisplayName = "Afternoon"),
    Dusk       UMETA(DisplayName = "Dusk"),
    Night      UMETA(DisplayName = "Night"),
    Midnight   UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.4f, 0.55f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientTemperature = 5500.0f;
};

/**
 * ADayNightCycleManager
 * Manages the full day/night cycle for the prehistoric survival game.
 * Controls sun position, fog density, sky light intensity, and colour temperature
 * across all time-of-day phases. Designed for Lumen GI compatibility.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Time Control ──────────────────────────────────────────────────────────

    /** Current time of day in hours (0-24) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeHours = 8.0f;

    /** Speed multiplier for time progression (1.0 = real time, 60.0 = 1 min = 1 hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0"))
    float TimeScale = 60.0f;

    /** Whether the cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    // ── Actor References ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<AActor> SunActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<AActor> FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<AActor> SkyLightActor;

    // ── Time of Day Presets ───────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings NightSettings;

    // ── Current State ─────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentPhase = ELight_TimeOfDay::Morning;

    // ── Blueprint Events ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintNativeEvent, Category = "Day Night Cycle")
    void OnPhaseChanged(ELight_TimeOfDay NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewHour);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentTimeHours() const { return CurrentTimeHours; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void ApplyCurrentLighting();

private:
    void AdvanceTime(float DeltaTime);
    void UpdateLighting();
    void UpdateSunPosition();
    void UpdateFog();
    void UpdateSkyLight();
    ELight_TimeOfDay CalculatePhase(float Hour) const;
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    FLight_TimeOfDaySettings GetSettingsForHour(float Hour) const;

    ELight_TimeOfDay LastPhase = ELight_TimeOfDay::Morning;
};
