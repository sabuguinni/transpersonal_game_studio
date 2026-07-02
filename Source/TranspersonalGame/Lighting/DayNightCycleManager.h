#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycleManager.generated.h"

/**
 * Prehistoric Day/Night Cycle Palettes
 * Each palette represents a distinct time of day with unique lighting characteristics.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    PreDawn     UMETA(DisplayName = "Pre-Dawn (3-5am)"),
    Dawn        UMETA(DisplayName = "Dawn (5-7am)"),
    Morning     UMETA(DisplayName = "Morning (7-10am)"),
    Midday      UMETA(DisplayName = "Midday (10am-2pm)"),
    Afternoon   UMETA(DisplayName = "Afternoon (2-5pm)"),
    GoldenHour  UMETA(DisplayName = "Golden Hour (5-7pm)"),
    Dusk        UMETA(DisplayName = "Dusk (7-8pm)"),
    EarlyNight  UMETA(DisplayName = "Early Night (8-10pm)"),
    MidNight    UMETA(DisplayName = "Midnight (10pm-2am)"),
    LateNight   UMETA(DisplayName = "Late Night (2-3am)")
};

/**
 * Lighting palette data for a specific time of day.
 * Defines all parameters needed to set the full atmospheric look.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_DayPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunPitchDegrees = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunYawDegrees = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.015f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogHeightFalloff = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
    FLinearColor SkyLightColor = FLinearColor(0.9f, 0.92f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Bloom")
    float BloomIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Exposure")
    float AutoExposureMin = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Exposure")
    float AutoExposureMax = 3.0f;
};

/**
 * ADayNightCycleManager
 * 
 * Manages the full 24-hour lighting cycle for the prehistoric world.
 * Smoothly interpolates between time-of-day palettes using Lumen-based
 * dynamic lighting. Drives sun position, fog density/color, sky light,
 * and post-process parameters.
 * 
 * Designed for a Cretaceous-era prehistoric survival game — each time
 * of day has a distinct emotional and survival implication for the player.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === TIME CONTROL ===

    /** Current time of day in hours (0.0 = midnight, 12.0 = noon, 23.99 = end of day) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeHours = 12.0f;

    /** Speed multiplier for time progression (1.0 = real-time, 60.0 = 1 min = 1 game hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time")
    float TimeScale = 60.0f;

    /** Whether the cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time")
    bool bCycleActive = true;

    // === LIGHTING REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    // === PALETTES ===

    /** All time-of-day palettes, indexed by ELight_TimeOfDay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Palettes")
    TMap<ELight_TimeOfDay, FLight_DayPalette> DayPalettes;

    // === BLUEPRINT EVENTS ===

    /** Called when the time of day transitions to a new phase */
    UFUNCTION(BlueprintImplementableEvent, Category = "DayNight|Events")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay, ELight_TimeOfDay PreviousTimeOfDay);

    /** Called every game hour */
    UFUNCTION(BlueprintImplementableEvent, Category = "DayNight|Events")
    void OnHourChanged(int32 NewHour);

    // === UTILITY ===

    /** Get the current time-of-day enum based on CurrentTimeHours */
    UFUNCTION(BlueprintCallable, Category = "DayNight|Utility")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    /** Get the current time formatted as "HH:MM" string */
    UFUNCTION(BlueprintCallable, Category = "DayNight|Utility")
    FString GetFormattedTime() const;

    /** Immediately jump to a specific time of day */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "DayNight|Control")
    void SetTimeOfDay(float NewTimeHours);

    /** Apply the palette for the current time immediately (no interpolation) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "DayNight|Control")
    void ApplyCurrentPalette();

private:
    /** Initialize default palettes for all times of day */
    void InitializeDefaultPalettes();

    /** Apply interpolated lighting between two palettes */
    void ApplyInterpolatedPalette(const FLight_DayPalette& A, const FLight_DayPalette& B, float Alpha);

    /** Get the palette for a specific time of day */
    FLight_DayPalette GetPaletteForTime(float TimeHours) const;

    /** Last tracked hour for OnHourChanged events */
    int32 LastHour = -1;

    /** Last tracked time-of-day phase for OnTimeOfDayChanged events */
    ELight_TimeOfDay LastTimeOfDay = ELight_TimeOfDay::Midday;
};
