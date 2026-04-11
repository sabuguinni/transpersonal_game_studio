#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EWeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Dusty       UMETA(DisplayName = "Dusty")
};

USTRUCT(BlueprintType)
struct FLightingConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.2f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereThickness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float VolumetricScatteringIntensity = 1.0f;
};

/**
 * Master lighting system manager that controls day/night cycle, weather, and atmospheric effects
 * Designed for prehistoric world with dynamic time of day and weather systems
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
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UStaticMeshComponent* SkyDome;

    // Time system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDayHours = 12.0f; // 0-24 hour format

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoAdvanceTime = true;

    // Weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAutoWeatherChanges = true;

    // Lighting configurations for different times/weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
    TMap<ETimeOfDay, FLightingConfiguration> TimeOfDayConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
    TMap<EWeatherType, FLightingConfiguration> WeatherConfigs;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(EWeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ETimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToConfiguration(const FLightingConfiguration& TargetConfig, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void EnableLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetVolumetricFogEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphericScattering();

protected:
    // Internal functions
    void UpdateSunPosition();
    void UpdateLightingConfiguration();
    void InterpolateLightingValues(const FLightingConfiguration& From, const FLightingConfiguration& To, float Alpha);
    void InitializeDefaultConfigurations();
    void HandleWeatherTransition(float DeltaTime);

    // Current interpolation state
    FLightingConfiguration CurrentConfig;
    FLightingConfiguration TargetConfig;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 2.0f;

    // Weather transition
    float WeatherTransitionTimer = 0.0f;
    float NextWeatherChangeTime = 300.0f; // 5 minutes default
};