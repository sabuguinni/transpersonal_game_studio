#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Light_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 5800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.78f, 0.86f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.78f, 0.86f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MultiScatteringFactor = 1.2f;

    FLight_AtmosphericSettings()
    {
        // Default Cretaceous period atmospheric settings
    }
};

/**
 * Manages atmospheric lighting and day/night cycle for the Cretaceous world
 * Handles Lumen global illumination, volumetric fog, and atmospheric scattering
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Atmospheric settings for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings")
    TMap<ELight_TimeOfDay, FLight_AtmosphericSettings> TimeOfDaySettings;

    // Current atmospheric state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Noon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    float TimeOfDayProgress = 0.5f; // 0.0 = start of period, 1.0 = end of period

    // Day/night cycle settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayNightCycleDuration = 1200.0f; // 20 minutes real time = 24 hours game time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float SunRotationSpeed = 15.0f; // Degrees per hour

    // Weather effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    // Lumen settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableLumenGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float LumenMaxLuminance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float LumenDiffuseColorBoost = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float LumenMaxDistance = 20000.0f;

    // Actor references (found at runtime)
    UPROPERTY(BlueprintReadOnly, Category = "Runtime References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime References")
    class ASkyLight* SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime References")
    class ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime References")
    class AExponentialHeightFog* FogActor;

public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay, float Progress = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SetWeatherIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SetCloudCoverage(float NewCoverage);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void EnableDayNightCycle(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ConfigureLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void RefreshAtmosphericActors();

    UFUNCTION(BlueprintPure, Category = "Atmosphere Info")
    FLight_AtmosphericSettings GetCurrentAtmosphericSettings() const;

    UFUNCTION(BlueprintPure, Category = "Atmosphere Info")
    float GetSunAngle() const;

    UFUNCTION(BlueprintPure, Category = "Atmosphere Info")
    bool IsNightTime() const;

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void SetupCretaceousAtmosphere();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void PreviewTimeOfDay();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void SaveAtmospherePreset();

protected:
    // Internal methods
    void UpdateDayNightCycle(float DeltaTime);
    void InterpolateAtmosphericSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha);
    void UpdateSunPosition();
    void UpdateWeatherEffects();
    void InitializeTimeOfDaySettings();
    void FindAtmosphericActors();
    FLight_AtmosphericSettings CalculateCurrentSettings() const;

private:
    float AccumulatedTime = 0.0f;
    bool bActorsInitialized = false;
};