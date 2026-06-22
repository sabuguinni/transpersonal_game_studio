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
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;
};

/**
 * ADayNightCycleManager
 * Manages the full day/night cycle for the prehistoric survival game.
 * Controls: DirectionalLight (sun), SkyAtmosphere, ExponentialHeightFog, SkyLight.
 * Uses Lumen GI for real-time global illumination.
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Time Control ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    float CurrentTimeOfDay = 12.0f;  // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    float DayDurationSeconds = 600.0f;  // 10 minutes = 1 full day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    bool bCycleEnabled = true;

    // ── Light References ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Lights")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Lights")
    AActor* SkyAtmosphereActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Lights")
    AActor* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Lights")
    AActor* FogActor = nullptr;

    // ── Time of Day Presets ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings NightSettings;

    // ── Current State ─────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle|State", meta=(AllowPrivateAccess="true"))
    ELight_TimeOfDay CurrentPhase = ELight_TimeOfDay::Midday;

    // ── Blueprint Callable ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void FindLightActorsInWorld();

private:
    void UpdateSunPosition(float TimeHours);
    void UpdateLightColors(float TimeHours);
    void UpdateFogSettings(float TimeHours);
    ELight_TimeOfDay TimeToPhase(float TimeHours) const;
    FLight_TimeOfDaySettings LerpSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    void InitDefaultPresets();
};
