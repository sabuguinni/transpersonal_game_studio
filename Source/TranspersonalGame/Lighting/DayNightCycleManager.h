#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycleManager.generated.h"

// ============================================================
// ELight_TimeOfDay — Prehistoric day phases
// ============================================================
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    DeepNight   UMETA(DisplayName = "Deep Night")
};

// ============================================================
// FLight_SunPhaseData — Per-phase lighting parameters
// ============================================================
USTRUCT(BlueprintType)
struct FLight_SunPhaseData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogInscatteringColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientIntensity = 0.3f;
};

// ============================================================
// ADayNightCycleManager — Controls the full prehistoric day/night cycle
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    // ── Tick ──────────────────────────────────────────────────
    virtual void Tick(float DeltaTime) override;

    // ── Public API ────────────────────────────────────────────

    /** Current time of day in hours (0-24) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeHours = 6.0f;

    /** How fast time passes (1.0 = real time, 60.0 = 1 min = 1 game hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeScale = 120.0f;

    /** Whether the cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    /** Reference to the directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    ADirectionalLight* SunLight = nullptr;

    /** Reference to the sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    ASkyLight* SkyLightActor = nullptr;

    /** Reference to the height fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    AExponentialHeightFog* HeightFog = nullptr;

    /** Current phase of day */
    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentPhase = ELight_TimeOfDay::Morning;

    /** Dawn phase data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Phases")
    FLight_SunPhaseData DawnData;

    /** Midday phase data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Phases")
    FLight_SunPhaseData MiddayData;

    /** Dusk phase data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Phases")
    FLight_SunPhaseData DuskData;

    /** Night phase data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Phases")
    FLight_SunPhaseData NightData;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewHours);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentTimeHours() const { return CurrentTimeHours; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void AutoFindLightingActors();

protected:
    virtual void BeginPlay() override;

private:
    void AdvanceTime(float DeltaTime);
    void UpdateSunPosition();
    void UpdateLightingForPhase(ELight_TimeOfDay Phase, float BlendAlpha);
    ELight_TimeOfDay ComputePhaseFromHour(float Hour) const;
    FLight_SunPhaseData LerpPhaseData(const FLight_SunPhaseData& A, const FLight_SunPhaseData& B, float Alpha) const;
    void ApplyPhaseData(const FLight_SunPhaseData& Data);

    float PreviousTimeHours = 6.0f;
};
