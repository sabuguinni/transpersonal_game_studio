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
struct FLight_TimeOfDaySettings
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
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogInscatteringColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
};

/**
 * ADayNightCycleManager
 * Manages the dynamic day/night cycle for the prehistoric survival world.
 * Controls sun position, sky atmosphere, volumetric fog, and ambient lighting
 * to create cinematic, emotionally resonant lighting for each time of day.
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

    /** Current time of day in hours (0.0 - 24.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 10.0f;

    /** Speed multiplier for time progression (1.0 = real time, 60.0 = 1 min per game hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0"))
    float TimeSpeed = 60.0f;

    /** Whether the cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    // === LIGHT REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ASkyLight* SkyLightActor = nullptr;

    // === TIME OF DAY PRESETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings NightSettings;

    // === FUNCTIONS ===

    /** Get current time of day enum based on hour */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    /** Set time directly (0-24 hours) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    /** Jump to a specific time of day preset */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void JumpToTimePreset(ELight_TimeOfDay TimePreset);

    /** Get formatted time string (HH:MM) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    FString GetFormattedTime() const;

    /** Is it currently daytime? */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    bool IsDaytime() const;

    /** Is it currently nighttime? */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    bool IsNighttime() const;

    /** Get sun intensity for current time */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    float GetCurrentSunIntensity() const;

private:
    void UpdateLighting(float DeltaTime);
    void ApplyLightingSettings(const FLight_TimeOfDaySettings& Settings);
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    void AutoFindLightActors();
    float GetSunPitchForTime(float Hour) const;
    float GetSunYawForTime(float Hour) const;
    float GetSunIntensityForTime(float Hour) const;
    FLinearColor GetSunColorForTime(float Hour) const;
    FLinearColor GetFogColorForTime(float Hour) const;
    float GetFogDensityForTime(float Hour) const;

    float AccumulatedTime = 0.0f;
};
