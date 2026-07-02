#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "DayNightCycleManager.generated.h"

/**
 * Day/Night Cycle phases for the prehistoric world.
 * Each phase has distinct lighting, atmosphere, and gameplay implications.
 */
UENUM(BlueprintType)
enum class ELight_DayPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Evening     UMETA(DisplayName = "Evening"),
    Night       UMETA(DisplayName = "Night"),
    DeepNight   UMETA(DisplayName = "Deep Night")
};

/**
 * Weather state affecting atmosphere and visibility.
 */
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Fog"),
    Haze        UMETA(DisplayName = "Haze")
};

/**
 * Lighting palette data for a specific time-of-day phase.
 * Drives DirectionalLight, SkyLight, and Fog parameters.
 */
USTRUCT(BlueprintType)
struct FLight_DayPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    ELight_DayPhase Phase = ELight_DayPhase::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunYaw = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SkyLightColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogHeightFalloff = 0.2f;

    /** Duration of this phase in real-time seconds (scaled by TimeScale) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float PhaseDurationSeconds = 300.0f;
};

/**
 * ADayNightCycleManager
 *
 * Manages the full 24-hour day/night cycle for the prehistoric world.
 * Controls DirectionalLight (sun/moon), SkyLight, ExponentialHeightFog,
 * and broadcasts phase-change events for gameplay systems to react to.
 *
 * Designed for Unreal Engine 5 with Lumen GI and Sky Atmosphere.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // =========================================================
    // CONFIGURATION
    // =========================================================

    /** How many real seconds = 1 in-game hour. Default 60s = 1 hour. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Config")
    float RealSecondsPerGameHour = 60.0f;

    /** Current in-game time (0.0 = midnight, 12.0 = noon, 24.0 = midnight again) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Config")
    float CurrentGameHour = 8.0f;

    /** Whether the cycle is running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Config")
    bool bCycleActive = true;

    /** Transition blend speed between palettes (0 = instant, 1 = smooth) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Config")
    float TransitionBlendSpeed = 2.0f;

    // =========================================================
    // LIGHT REFERENCES
    // =========================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Lights")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Lights")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Lights")
    AExponentialHeightFog* FogActor = nullptr;

    // =========================================================
    // PALETTES
    // =========================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Palettes")
    TArray<FLight_DayPalette> DayPalettes;

    // =========================================================
    // STATE (READ-ONLY)
    // =========================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DayNight|State")
    ELight_DayPhase CurrentPhase = ELight_DayPhase::Morning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DayNight|State")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DayNight|State")
    float CurrentSunPitch = -45.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DayNight|State")
    float CurrentSunYaw = 180.0f;

    // =========================================================
    // BLUEPRINT EVENTS
    // =========================================================

    UFUNCTION(BlueprintImplementableEvent, Category = "DayNight|Events")
    void OnPhaseChanged(ELight_DayPhase NewPhase, ELight_DayPhase OldPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "DayNight|Events")
    void OnWeatherChanged(ELight_WeatherState NewWeather);

    // =========================================================
    // PUBLIC API
    // =========================================================

    /** Set the current game hour directly (0-24) */
    UFUNCTION(BlueprintCallable, Category = "DayNight")
    void SetGameHour(float NewHour);

    /** Force a specific phase immediately (no blend) */
    UFUNCTION(BlueprintCallable, Category = "DayNight")
    void SetPhaseImmediate(ELight_DayPhase Phase);

    /** Get the current phase as a display string */
    UFUNCTION(BlueprintCallable, Category = "DayNight")
    FString GetCurrentPhaseString() const;

    /** Get normalized time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UFUNCTION(BlueprintCallable, Category = "DayNight")
    float GetNormalizedTimeOfDay() const;

    /** Is it currently night (for gameplay — dinosaur behaviour changes) */
    UFUNCTION(BlueprintCallable, Category = "DayNight")
    bool IsNightTime() const;

    /** Is it currently dangerous visibility (night or heavy fog) */
    UFUNCTION(BlueprintCallable, Category = "DayNight")
    bool IsLowVisibility() const;

    UFUNCTION(CallInEditor, Category = "DayNight")
    void ApplyCurrentPaletteInEditor();

protected:
    void InitializeDefaultPalettes();
    void AutoFindLightActors();
    void UpdateLighting(float DeltaTime);
    void ApplyPalette(const FLight_DayPalette& Palette, float BlendAlpha);
    ELight_DayPhase HourToPhase(float Hour) const;
    const FLight_DayPalette* FindPaletteForPhase(ELight_DayPhase Phase) const;

private:
    ELight_DayPhase PreviousPhase = ELight_DayPhase::Morning;
    float PhaseBlendAlpha = 1.0f;
    FLight_DayPalette CurrentBlendedPalette;
    FLight_DayPalette TargetPalette;
};
