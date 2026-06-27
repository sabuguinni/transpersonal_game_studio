#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "PrehistoricAtmosphereSystem.generated.h"

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
    Midnight    UMETA(DisplayName = "Midnight")
};

// ============================================================
// ELight_WeatherState — Prehistoric weather conditions
// ============================================================
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

// ============================================================
// FLight_DayPhaseSettings — Per-phase lighting parameters
// ============================================================
USTRUCT(BlueprintType)
struct FLight_DayPhaseSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.8f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float BloomIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureMin = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureMax = 3.0f;

    FLight_DayPhaseSettings() {}
};

// ============================================================
// APrehistoricAtmosphereSystem — Main lighting director actor
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Prehistoric Atmosphere System"))
class TRANSPERSONALGAME_API APrehistoricAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricAtmosphereSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ---- Day/Night Cycle ----

    /** Total length of one full day in real seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayLengthSeconds = 600.0f;

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentTimeNormalized = 0.35f;

    /** Whether the day/night cycle is actively ticking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bCycleActive = true;

    /** Current phase of day */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Morning;

    // ---- Weather ----

    /** Current weather state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    /** Weather transition speed (0=instant, 1=slow) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.5f;

    /** Probability of rain starting per minute (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainProbabilityPerMinute = 0.05f;

    // ---- Scene References ----

    /** Reference to the directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ADirectionalLight* SunLight = nullptr;

    /** Reference to the height fog actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    AExponentialHeightFog* HeightFog = nullptr;

    /** Reference to the sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ASkyLight* SceneSkyLight = nullptr;

    // ---- Day Phase Settings ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Settings")
    FLight_DayPhaseSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Settings")
    FLight_DayPhaseSettings MorningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Settings")
    FLight_DayPhaseSettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Settings")
    FLight_DayPhaseSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase Settings")
    FLight_DayPhaseSettings NightSettings;

    // ---- Blueprint Events ----

    UFUNCTION(BlueprintImplementableEvent, Category = "Atmosphere")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Atmosphere")
    void OnWeatherChanged(ELight_WeatherState NewWeather);

    // ---- Public Methods ----

    /** Set time of day directly (0-1 normalized) */
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NormalizedTime);

    /** Transition to a new weather state */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(ELight_WeatherState NewWeather);

    /** Get current time as hours (0-24) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day/Night Cycle")
    float GetCurrentHour() const;

    /** Get current time phase label */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day/Night Cycle")
    FString GetTimeOfDayLabel() const;

    /** Force auto-discover scene lighting actors */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void AutoDiscoverLightingActors();

private:
    void AdvanceDayCycle(float DeltaTime);
    void ApplyLightingForTime(float NormalizedTime);
    void UpdateTimeOfDayEnum(float NormalizedTime);
    FLight_DayPhaseSettings InterpolatePhaseSettings(
        const FLight_DayPhaseSettings& A,
        const FLight_DayPhaseSettings& B,
        float Alpha) const;
    void ApplyPhaseSettings(const FLight_DayPhaseSettings& Settings);

    ELight_TimeOfDay PreviousTimeOfDay = ELight_TimeOfDay::Morning;
    float WeatherTransitionAlpha = 0.0f;
    ELight_WeatherState TargetWeather = ELight_WeatherState::Clear;
};
