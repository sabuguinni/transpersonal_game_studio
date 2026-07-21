#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "DayNightCycleComponent.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
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
struct FLight_TimeOfDayPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;
};

/**
 * ULight_DayNightCycleComponent
 * Drives the full day/night cycle for the prehistoric survival world.
 * Manages sun position, fog color, sky light, and atmosphere transitions.
 * Attach to a persistent actor (e.g., GameMode or WorldSettings actor).
 */
UCLASS(ClassGroup = (Lighting), meta = (BlueprintSpawnableComponent), DisplayName = "Day Night Cycle Component")
class TRANSPERSONALGAME_API ULight_DayNightCycleComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_DayNightCycleComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Public API ---

    /** Advance time by DeltaSeconds (real seconds mapped to game hours) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void AdvanceTime(float DeltaSeconds);

    /** Set the current time of day directly (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NormalizedTime);

    /** Get the current normalized time (0.0-1.0) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    float GetNormalizedTime() const { return CurrentNormalizedTime; }

    /** Get the current time of day enum */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    /** Get current time as hours (0-24) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    float GetCurrentHour() const { return CurrentNormalizedTime * 24.0f; }

    /** Is it currently nighttime? */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    bool IsNighttime() const;

    /** Is it currently daytime? */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    bool IsDaytime() const;

    /** Pause/resume the cycle */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetCyclePaused(bool bPaused) { bCyclePaused = bPaused; }

    // --- Settings ---

    /** How many real seconds equal one full in-game day */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings")
    float RealSecondsPerGameDay = 1200.0f; // 20 real minutes = 1 game day

    /** Starting time (normalized 0-1, 0.25 = 6am) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings")
    float StartingNormalizedTime = 0.25f;

    /** Whether the cycle is currently paused */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings")
    bool bCyclePaused = false;

    /** Enable Lumen sky light real-time capture updates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings")
    bool bEnableLumenSkyCapture = true;

    // --- Light References (set in editor or auto-found) ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    ASkyLight* SkyLightActor = nullptr;

    // --- Palettes ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_TimeOfDayPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_TimeOfDayPalette MorningPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_TimeOfDayPalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_TimeOfDayPalette AfternoonPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_TimeOfDayPalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_TimeOfDayPalette TwilightPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_TimeOfDayPalette NightPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_TimeOfDayPalette DeepNightPalette;

private:
    float CurrentNormalizedTime = 0.25f;
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Morning;

    void UpdateLighting(float NormalizedTime);
    void ApplyPaletteToWorld(const FLight_TimeOfDayPalette& Palette);
    FLight_TimeOfDayPalette LerpPalettes(const FLight_TimeOfDayPalette& A, const FLight_TimeOfDayPalette& B, float Alpha) const;
    ELight_TimeOfDay NormalizedTimeToEnum(float NormalizedTime) const;
    void AutoFindLightActors();
    void InitializeDefaultPalettes();
};
