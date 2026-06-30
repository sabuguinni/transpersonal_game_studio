#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "DayNightCycleManager.generated.h"

// ============================================================
// ELight_TimeOfDay — Lighting phase enum
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
    Midnight    UMETA(DisplayName = "Midnight")
};

// ============================================================
// FLight_DayPhaseSettings — Per-phase lighting configuration
// ============================================================
USTRUCT(BlueprintType)
struct FLight_DayPhaseSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    ELight_TimeOfDay Phase = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    float SunPitchDegrees = -75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    float SunYawOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    FLinearColor FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    FLinearColor SkyLightColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    float TransitionDurationSeconds = 120.0f;
};

// ============================================================
// ADayNightCycleManager — Main lighting manager actor
// ============================================================
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
    // ---- References ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    // ---- Cycle Settings ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    float FullDayCycleDurationSeconds = 1200.0f; // 20 real minutes = 1 game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    float CurrentTimeOfDayNormalized = 0.5f; // 0=midnight, 0.5=noon, 1=midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    bool bCycleEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    float TimeScale = 1.0f;

    // ---- Phase Settings Array ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phases")
    TArray<FLight_DayPhaseSettings> PhaseSettings;

    // ---- Current State ----
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|State", meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentPhase = ELight_TimeOfDay::Midday;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|State", meta = (AllowPrivateAccess = "true"))
    float CurrentSunPitch = -75.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|State", meta = (AllowPrivateAccess = "true"))
    float CurrentSunYaw = 0.0f;

    // ---- Blueprint Events ----
    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
    void OnPhaseChanged(ELight_TimeOfDay NewPhase, ELight_TimeOfDay OldPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
    void OnDawnBegins();

    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
    void OnDuskBegins();

    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
    void OnNightBegins();

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void SetPhaseImmediate(ELight_TimeOfDay Phase);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void SetCycleEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void SetTimeScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Query")
    ELight_TimeOfDay GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Lighting|Query")
    float GetCurrentTimeNormalized() const { return CurrentTimeOfDayNormalized; }

    UFUNCTION(BlueprintCallable, Category = "Lighting|Query")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|Query")
    float GetSunriseProgress() const;

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyDawnPreset();

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyDuskPreset();

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyNightPreset();

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyMiddayPreset();

private:
    void AutoDiscoverLightActors();
    void UpdateSunPosition(float DeltaTime);
    void ApplyPhaseSettings(const FLight_DayPhaseSettings& Settings);
    void InterpolateToPhase(const FLight_DayPhaseSettings& From, const FLight_DayPhaseSettings& To, float Alpha);
    ELight_TimeOfDay ComputePhaseFromTime(float NormalizedTime) const;
    void InitializeDefaultPhases();

    float TransitionAlpha = 0.0f;
    ELight_TimeOfDay PreviousPhase = ELight_TimeOfDay::Midday;
    bool bTransitioning = false;
};
