#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn (05:00-07:00)"),
    Morning     UMETA(DisplayName = "Morning (07:00-10:00)"),
    Midday      UMETA(DisplayName = "Midday (10:00-14:00)"),
    Afternoon   UMETA(DisplayName = "Afternoon (14:00-17:00)"),
    Dusk        UMETA(DisplayName = "Dusk (17:00-19:00)"),
    Night       UMETA(DisplayName = "Night (19:00-05:00)")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Dense Fog")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe - Warm lighting"),
    Cautious    UMETA(DisplayName = "Cautious - Neutral lighting"),
    Danger      UMETA(DisplayName = "Danger - Cool, harsh lighting"),
    Terror      UMETA(DisplayName = "Terror - Dramatic shadows")
};

USTRUCT(BlueprintType)
struct FLightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevation = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAzimuth = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyTint = FLinearColor(0.5f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHaziness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;
};

/**
 * Central manager for all lighting and atmospheric effects
 * Handles day/night cycle, weather systems, and emotional lighting states
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ASkyLight* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class AVolumetricCloud* VolumetricClouds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class AExponentialHeightFog* HeightFog;

public:
    // Time and weather control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeather = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional")
    EThreatLevel CurrentThreatLevel = EThreatLevel::Safe;

    // Lighting presets for different times and conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ETimeOfDay, FLightingPreset> TimePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EWeatherState, FLightingPreset> WeatherModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EThreatLevel, FLightingPreset> ThreatModifiers;

    // Dynamic lighting functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(EWeatherState NewWeather, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional")
    void SetThreatLevel(EThreatLevel NewThreatLevel, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting();

    UFUNCTION(BlueprintPure, Category = "Time")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;

    // Interior lighting support
    UFUNCTION(BlueprintCallable, Category = "Interior")
    void EnterInteriorSpace(const FLightingPreset& InteriorPreset);

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void ExitInteriorSpace();

    // Emergency lighting states for specific gameplay moments
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerDinosauerEncounterLighting();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerStormLighting();

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void RestoreNormalLighting();

private:
    void InitializeLightingComponents();
    void SetupDefaultPresets();
    void ApplyLightingPreset(const FLightingPreset& Preset, float BlendTime = 1.0f);
    FLightingPreset CalculateCurrentLightingState();
    void UpdateSunPosition();
    void UpdateAtmosphericEffects();

    // Transition system
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 1.0f;
    FLightingPreset TransitionStartPreset;
    FLightingPreset TransitionTargetPreset;

    // Interior state
    bool bIsInInterior = false;
    FLightingPreset ExteriorStateBeforeInterior;
};