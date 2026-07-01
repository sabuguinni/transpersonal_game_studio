#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "DayNightCycleManager.generated.h"

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

USTRUCT(BlueprintType)
struct FLight_TimeOfDayPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunYaw = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SkyLightColor = FLinearColor(0.75f, 0.82f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float BloomIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float ExposureBias = 0.0f;
};

/**
 * ADayNightCycleManager
 * Manages the full day/night cycle for the prehistoric survival world.
 * Controls sun position, fog, sky, and post-process for each time of day.
 * Designed for Unreal Engine 5 with Lumen GI and SkyAtmosphere.
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
    // === TIME CONTROL ===

    /** Current time of day in hours (0.0 = midnight, 12.0 = noon, 24.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 6.5f;

    /** Speed multiplier for time progression (1.0 = real-time, 60.0 = 1 min = 1 game hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time", meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float TimeSpeedMultiplier = 120.0f;

    /** Whether the day/night cycle is actively ticking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time")
    bool bCycleActive = true;

    /** Duration of a full day in real-world seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time", meta = (ClampMin = "60.0"))
    float DayDurationSeconds = 720.0f;

    // === SCENE REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    ASkyLight* SkyLightActor = nullptr;

    // === PALETTE PRESETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Palettes")
    FLight_TimeOfDayPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Palettes")
    FLight_TimeOfDayPalette MorningPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Palettes")
    FLight_TimeOfDayPalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Palettes")
    FLight_TimeOfDayPalette AfternoonPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Palettes")
    FLight_TimeOfDayPalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Palettes")
    FLight_TimeOfDayPalette NightPalette;

    // === CURRENT STATE ===

    UPROPERTY(BlueprintReadOnly, Category = "DayNight|State")
    ELight_TimeOfDay CurrentPhase = ELight_TimeOfDay::Dawn;

    UPROPERTY(BlueprintReadOnly, Category = "DayNight|State")
    float CurrentSunPitchDegrees = -8.0f;

    UPROPERTY(BlueprintReadOnly, Category = "DayNight|State")
    float CurrentSunYawDegrees = -90.0f;

    // === BLUEPRINT FUNCTIONS ===

    /** Force a specific time of day immediately */
    UFUNCTION(BlueprintCallable, Category = "DayNight")
    void SetTimeOfDay(float NewHour);

    /** Get the current time phase */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DayNight")
    ELight_TimeOfDay GetCurrentPhase() const;

    /** Get formatted time string (e.g., "06:30") */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DayNight")
    FString GetFormattedTime() const;

    /** Manually trigger auto-discovery of scene lighting actors */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "DayNight")
    void AutoDiscoverLightingActors();

    /** Apply a specific palette preset immediately (for testing) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "DayNight")
    void ApplyDawnPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "DayNight")
    void ApplyMiddayPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "DayNight")
    void ApplyDuskPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "DayNight")
    void ApplyNightPreset();

private:
    void InitializeDefaultPalettes();
    void UpdateCycle(float DeltaTime);
    void ApplyPalette(const FLight_TimeOfDayPalette& Palette);
    FLight_TimeOfDayPalette InterpolatePalettes(const FLight_TimeOfDayPalette& A, const FLight_TimeOfDayPalette& B, float Alpha) const;
    ELight_TimeOfDay TimeToPhase(float Hour) const;
    float HourToSunPitch(float Hour) const;
    float HourToSunYaw(float Hour) const;
    void FindLightingActorsInWorld();
};
