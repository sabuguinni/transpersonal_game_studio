#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "DayNightCycle.generated.h"

// Light_DayPhase: Enum for time-of-day phases
UENUM(BlueprintType)
enum class ELight_DayPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

// Light_SunSettings: Per-phase sun configuration
USTRUCT(BlueprintType)
struct FLight_SunSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.4f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;
};

/**
 * ADayNightCycle — Dynamic day/night cycle manager for the Cretaceous world.
 * Controls sun position, color temperature, fog density, and sky light
 * across 8 time-of-day phases. Uses Lumen GI for real-time bounce light.
 *
 * Cinematic intent: Roger Deakins approach — light serves emotional narrative.
 * Dawn = hope/danger, Midday = false safety, Dusk = urgency, Night = terror.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Lighting")
class TRANSPERSONALGAME_API ADayNightCycle : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycle();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // === REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AActor* FogActor;

    // === TIME SETTINGS ===

    /** Current time of day in hours (0.0 = midnight, 12.0 = noon, 24.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 10.0f;

    /** Speed multiplier for day/night cycle (1.0 = real time, 60.0 = 1 min per game hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time", meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float TimeScale = 60.0f;

    /** Whether the cycle is currently running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    bool bCycleActive = true;

    // === CURRENT STATE ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State")
    ELight_DayPhase CurrentPhase = ELight_DayPhase::Morning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State")
    float CurrentSunPitch = -45.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State")
    float CurrentSunIntensity = 10.0f;

    // === PHASE SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phases")
    TMap<ELight_DayPhase, FLight_SunSettings> PhaseSettings;

    // === FUNCTIONS ===

    /** Set time of day directly (0-24 hours) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hour);

    /** Get current phase based on hour */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_DayPhase GetPhaseForHour(float Hour) const;

    /** Apply lighting settings for a specific phase */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyPhaseSettings(ELight_DayPhase Phase);

    /** Interpolate between two phase settings */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_SunSettings InterpolateSettings(const FLight_SunSettings& A, const FLight_SunSettings& B, float Alpha) const;

    /** Force immediate lighting update (call after changing CurrentHour) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ForceUpdateLighting();

    /** Get time as formatted string (HH:MM) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FString GetTimeString() const;

private:
    void InitializeDefaultPhaseSettings();
    void UpdateSunPosition(float Hour);
    void UpdateFogSettings(const FLight_SunSettings& Settings);
    void UpdateSkyLight(const FLight_SunSettings& Settings);

    float PreviousHour = -1.0f;
};
