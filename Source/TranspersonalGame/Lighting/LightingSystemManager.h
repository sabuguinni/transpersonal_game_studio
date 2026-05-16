#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Engine/VolumetricCloud.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "LightingSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FRotator SunRotation = FRotator(-45.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    FLight_TimeOfDaySettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
        SunTemperature = 5500.0f;
        SunRotation = FRotator(-45.0f, 45.0f, 0.0f);
        RayleighScattering = 0.8f;
        MieScattering = 0.02f;
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor WeatherTint = FLinearColor::White;

    FLight_WeatherSettings()
    {
        CloudCoverage = 0.0f;
        RainIntensity = 0.0f;
        WindStrength = 1.0f;
        WeatherTint = FLinearColor::White;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULightingSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULightingSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core lighting management
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void InitializeLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void ApplyAtmosphericCorrection();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetTimeOfDay(float TimeHours);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetWeatherConditions(const FLight_WeatherSettings& WeatherSettings);

    // Light actor management
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void FindOrCreateSunLight();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void FindOrCreateSkyAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void FindOrCreateFog();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void ConfigurePostProcessing();

    // Biome-specific lighting
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void ApplyBiomeLighting(EBiomeType BiomeType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetInteriorLighting(bool bIsInterior, float AmbientMultiplier = 0.3f);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Lighting System")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting System")
    FLight_TimeOfDaySettings GetCurrentLightingSettings() const { return CurrentLightingSettings; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Lighting System")
    float CurrentTimeOfDay = 12.0f; // Noon

    UPROPERTY(BlueprintReadOnly, Category = "Lighting System")
    FLight_TimeOfDaySettings CurrentLightingSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting System")
    FLight_WeatherSettings CurrentWeatherSettings;

    // Actor references
    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    APostProcessVolume* PostProcessVolume;

private:
    void UpdateSunLighting();
    void UpdateAtmosphere();
    void UpdateFog();
    void UpdatePostProcessing();
    
    FLight_TimeOfDaySettings GetBiomeLightingSettings(EBiomeType BiomeType) const;
    FLinearColor InterpolateSunColor(float TimeHours) const;
    float InterpolateSunIntensity(float TimeHours) const;
    FRotator CalculateSunRotation(float TimeHours) const;
};