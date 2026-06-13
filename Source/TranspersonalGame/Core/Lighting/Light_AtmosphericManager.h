#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Light_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FColor SunColor = FColor(255, 240, 200, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FColor SkyLightColor = FColor(200, 220, 255, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FColor FogInscatteringColor = FColor(255, 230, 180, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FColor RayleighScattering = FColor(84, 140, 255, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighExponentialDistribution = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FColor MieScattering = FColor(255, 230, 204, 255);

    FLight_AtmosphericSettings()
    {
        // Default Cretaceous period atmospheric conditions
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ULight_AtmosphericManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    void SetAtmosphericSettings(const FLight_AtmosphericSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    FLight_AtmosphericSettings GetCurrentAtmosphericSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    void UpdateTimeOfDay(float TimeOfDayHours);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    void SetWeatherCondition(EWeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager", CallInEditor)
    void RefreshAtmosphericComponents();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<class ASkyAtmosphere> SkyAtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<class ADirectionalLight> DirectionalLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<class ASkyLight> SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<class AExponentialHeightFog> HeightFogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<class APostProcessVolume> PostProcessVolumeActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_AtmosphericSettings CurrentSettings;

private:
    void FindOrCreateAtmosphericActors();
    void ConfigureSkyAtmosphere();
    void ConfigureDirectionalLight();
    void ConfigureSkyLight();
    void ConfigureHeightFog();
    void ConfigurePostProcessVolume();
    void SaveAtmosphericState();

    UPROPERTY()
    float CurrentTimeOfDay = 12.0f; // Noon by default

    UPROPERTY()
    EWeatherType CurrentWeather = EWeatherType::Clear;
};