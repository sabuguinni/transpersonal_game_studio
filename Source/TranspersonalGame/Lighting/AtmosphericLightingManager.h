#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "AtmosphericLightingManager.generated.h"

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
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.96f, 0.88f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.003996f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.447f, 0.638f, 1.0f, 1.0f);

    FLight_AtmosphericSettings()
    {
        // Default Cretaceous tropical atmosphere values
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_DayNightCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f; // Real-time minutes per game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoAdvanceTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevationAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAzimuthAngle = 180.0f;

    FLight_DayNightCycle()
    {
        // Default midday values
    }
};

/**
 * Atmospheric Lighting Manager - Controls dynamic day/night cycle and weather systems
 * Manages Lumen global illumination, volumetric lighting, and atmospheric scattering
 * Optimized for Cretaceous period prehistoric environments
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAtmosphericLightingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAtmosphericLightingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core atmospheric functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void InitializeAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetWeatherType(ELight_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    // Lumen configuration
    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void ConfigureLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void EnableVolumetricLighting();

    // Lighting actor management
    UFUNCTION(BlueprintCallable, Category = "Lighting Actors")
    void FindAndConfigureLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting Actors")
    void UpdateDirectionalLight();

    UFUNCTION(BlueprintCallable, Category = "Lighting Actors")
    void UpdateSkyAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting Actors")
    void UpdateExponentialHeightFog();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    float GetCurrentTimeOfDay() const { return DayNightCycle.TimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    ELight_TimeOfDay GetTimeOfDayEnum() const;

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    ELight_WeatherType GetCurrentWeatherType() const { return CurrentWeatherType; }

protected:
    // Current atmospheric state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    FLight_DayNightCycle DayNightCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeatherType = ELight_WeatherType::Clear;

    // Lighting actor references
    UPROPERTY()
    TObjectPtr<ADirectionalLight> DirectionalLightActor;

    UPROPERTY()
    TObjectPtr<AActor> SkyAtmosphereActor;

    UPROPERTY()
    TObjectPtr<AActor> ExponentialHeightFogActor;

    UPROPERTY()
    TObjectPtr<AActor> SkyLightActor;

    UPROPERTY()
    TObjectPtr<APostProcessVolume> PostProcessVolumeActor;

private:
    // Internal update functions
    void UpdateSunPosition();
    void UpdateAtmosphericScattering();
    void UpdateFogSettings();
    void UpdatePostProcessEffects();

    // Utility functions
    FLinearColor CalculateSunColorByTime(float TimeOfDay) const;
    float CalculateSunIntensityByTime(float TimeOfDay) const;
    FRotator CalculateSunRotationByTime(float TimeOfDay) const;
};