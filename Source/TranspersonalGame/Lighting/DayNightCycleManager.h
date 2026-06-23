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
 * Cretaceous time-of-day phases with emotional intent
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    MorningGold UMETA(DisplayName = "Morning Golden Hour"),
    Midday      UMETA(DisplayName = "Midday"),
    AfternoonGold UMETA(DisplayName = "Afternoon Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

/**
 * Per-phase lighting configuration
 */
USTRUCT(BlueprintType)
struct FLight_TimeOfDayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogInscatteringColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;
};

/**
 * ADayNightCycleManager
 * Drives the Cretaceous day/night cycle by animating DirectionalLight pitch,
 * fog density, and sky light intensity over time.
 * Designed to work with Lumen GI — no baked lighting required.
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
    // ─── Configuration ───────────────────────────────────────────────────────

    /** Total duration of a full day/night cycle in real seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationSeconds = 600.0f;

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentTimeNormalized = 0.25f;

    /** Whether the cycle is running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    /** Reference to the main directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    ADirectionalLight* SunLight = nullptr;

    /** Reference to the exponential height fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    AExponentialHeightFog* HeightFog = nullptr;

    /** Reference to the sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    ASkyLight* SceneSkyLight = nullptr;

    // ─── Per-Phase Configs ────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Phases")
    FLight_TimeOfDayConfig DawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Phases")
    FLight_TimeOfDayConfig MorningGoldConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Phases")
    FLight_TimeOfDayConfig MiddayConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Phases")
    FLight_TimeOfDayConfig AfternoonGoldConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Phases")
    FLight_TimeOfDayConfig DuskConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Phases")
    FLight_TimeOfDayConfig NightConfig;

    // ─── Runtime State ────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentPhase = ELight_TimeOfDay::MorningGold;

    // ─── Blueprint Events ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Cycle")
    void OnPhaseChanged(ELight_TimeOfDay NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentTimeNormalized() const { return CurrentTimeNormalized; }

    UFUNCTION(CallInEditor, Category = "Day Night Cycle")
    void AutoFindLightActors();

private:
    void ApplyLightingConfig(const FLight_TimeOfDayConfig& Config);
    FLight_TimeOfDayConfig LerpConfigs(const FLight_TimeOfDayConfig& A, const FLight_TimeOfDayConfig& B, float Alpha);
    FLight_TimeOfDayConfig GetConfigForTime(float NormalizedTime);
    void InitDefaultPhaseConfigs();

    ELight_TimeOfDay PreviousPhase = ELight_TimeOfDay::MorningGold;
};
