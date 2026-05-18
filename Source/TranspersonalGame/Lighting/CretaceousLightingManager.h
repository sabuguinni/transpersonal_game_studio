#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "CretaceousLightingManager.generated.h"

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

UENUM(BlueprintType)
enum class ELight_WeatherCondition : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Humid       UMETA(DisplayName = "Humid")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 135.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
};

/**
 * Manages atmospheric lighting and weather systems for the Cretaceous period setting.
 * Handles day/night cycles, weather transitions, and prehistoric atmospheric conditions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCretaceousLightingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCretaceousLightingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core lighting functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherCondition(ELight_WeatherCondition NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphericSettings(const FLight_AtmosphericSettings& NewSettings);

    // Day/night cycle
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void StartDayNightCycle(float CycleDurationMinutes = 24.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void StopDayNightCycle();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentTimeOfDayNormalized() const { return CurrentTimeNormalized; }

    // Weather system
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherCondition TargetWeather, float TransitionDuration = 5.0f);

    // Interior lighting
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureInteriorLighting(AActor* InteriorVolume, bool bEnableFirelight = true);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVolumetricCloudsEnabled(bool bEnabled);

protected:
    // Lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<ASkyAtmosphere> SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TObjectPtr<APostProcessVolume> PostProcessVolume;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_WeatherCondition CurrentWeather = ELight_WeatherCondition::Clear;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FLight_AtmosphericSettings CurrentSettings;

    // Day/night cycle
    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    bool bDayNightCycleActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    float CurrentTimeNormalized = 0.5f; // 0.0 = midnight, 0.5 = noon

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    float CycleDuration = 24.0f; // Minutes

    // Weather transition
    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    bool bWeatherTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float WeatherTransitionProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    ELight_WeatherCondition TargetWeather = ELight_WeatherCondition::Clear;

private:
    // Internal update functions
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void ApplyTimeOfDayLighting();
    void ApplyWeatherEffects();
    
    // Actor finding/creation
    void FindOrCreateLightingActors();
    ADirectionalLight* FindOrCreateSunLight();
    ASkyAtmosphere* FindOrCreateSkyAtmosphere();
    AExponentialHeightFog* FindOrCreateHeightFog();
    APostProcessVolume* FindOrCreatePostProcessVolume();

    // Preset configurations
    FLight_AtmosphericSettings GetTimeOfDaySettings(ELight_TimeOfDay TimeOfDay) const;
    FLight_AtmosphericSettings GetWeatherSettings(ELight_WeatherCondition Weather) const;

    // Timer handles
    FTimerHandle DayNightCycleTimer;
    FTimerHandle WeatherTransitionTimer;
};