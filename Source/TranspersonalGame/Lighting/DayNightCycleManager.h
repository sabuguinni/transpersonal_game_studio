#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "DayNightCycleManager.generated.h"

/**
 * Cretaceous Day/Night Cycle Manager
 * Controls sun position, sky color, fog density, and ambient light
 * throughout a full 24-hour prehistoric day cycle.
 * 
 * Lighting philosophy: light serves emotional intent, not just rendering.
 * Each phase has a distinct mood — dawn = hope, midday = danger, dusk = tension, night = terror.
 */

UENUM(BlueprintType)
enum class ELight_DayPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Twilight    UMETA(DisplayName = "Twilight"),
    Night       UMETA(DisplayName = "Night"),
    DeepNight   UMETA(DisplayName = "Deep Night")
};

USTRUCT(BlueprintType)
struct FLight_DayPhaseSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.55f, 0.72f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientOcclusionIntensity = 0.5f;
};

UCLASS(ClassGroup = (Lighting), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    /** Current time of day in hours (0.0 - 24.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 8.0f;

    /** Speed multiplier for time progression (1.0 = real time, 60.0 = 1 min per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float TimeScale = 60.0f;

    /** Whether the cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bCycleActive = true;

    /** Full day duration in real seconds (default: 20 minutes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float RealDayDurationSeconds = 1200.0f;

    /** Reference to the scene's directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight = nullptr;

    /** Reference to the scene's exponential height fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* SceneFog = nullptr;

    /** Reference to the scene's sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ASkyLight* SceneSkyLight = nullptr;

    /** Current day phase */
    UPROPERTY(BlueprintReadOnly, Category = "Day/Night Cycle", meta = (AllowPrivateAccess = "true"))
    ELight_DayPhase CurrentPhase = ELight_DayPhase::Morning;

    /** Phase settings for each time of day */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Settings")
    TMap<ELight_DayPhase, FLight_DayPhaseSettings> PhaseSettings;

    /** Get the current day phase based on time */
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    ELight_DayPhase GetPhaseForTime(float TimeOfDay) const;

    /** Get interpolated settings between two phases */
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    FLight_DayPhaseSettings GetInterpolatedSettings(float TimeOfDay) const;

    /** Apply lighting settings to scene actors */
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void ApplyLightingSettings(const FLight_DayPhaseSettings& Settings);

    /** Manually set time of day */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NewTime);

    /** Auto-discover scene lighting actors */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day/Night Cycle")
    void AutoDiscoverLightingActors();

private:
    void InitializeDefaultPhaseSettings();
    float GetSunPitchForTime(float TimeOfDay) const;
    float GetSunYawForTime(float TimeOfDay) const;
    FLinearColor LerpColor(const FLinearColor& A, const FLinearColor& B, float Alpha) const;
};
