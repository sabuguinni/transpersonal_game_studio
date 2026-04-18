#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "LightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Sunset      UMETA(DisplayName = "Sunset"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    FLight_TimeOfDaySettings()
    {
        SunIntensity = 10.0f;
        SunColor = FLinearColor::White;
        SunRotation = FRotator(-45.0f, 0.0f, 0.0f);
        SkyLightIntensity = 1.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    }
};

/**
 * Manages dynamic lighting, day/night cycle, and atmospheric effects for the prehistoric world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ASkyLight* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class AExponentialHeightFog* AtmosphericFog;

    // Day/Night cycle settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimePhase = ELight_TimeOfDay::Midday;

    // Time of day presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<ELight_TimeOfDay, FLight_TimeOfDaySettings> TimeOfDayPresets;

    // Weather effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity = 1.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLumenGlobalIllumination = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableVolumetricClouds = true;

public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimePhase(ELight_TimeOfDay NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(ELight_TimeOfDay TimePhase);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimePhase() const { return CurrentTimePhase; }

private:
    // Internal methods
    void InitializeTimeOfDayPresets();
    void UpdateDayNightCycle(float DeltaTime);
    void InterpolateLightingSettings(const FLight_TimeOfDaySettings& FromSettings, const FLight_TimeOfDaySettings& ToSettings, float Alpha);
    ELight_TimeOfDay CalculateTimePhase(float TimeOfDay);
    void FindOrCreateLightingActors();
    void ConfigureLumenSettings();
    void UpdateSunPosition();
    void UpdateAtmosphericEffects();

    // Helper variables
    float TimeAccumulator = 0.0f;
    bool bLightingActorsInitialized = false;
};