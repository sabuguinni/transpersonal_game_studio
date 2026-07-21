#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "SharedTypes.h"
#include "Light_WeatherSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector ZoneLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EBiomeType WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightIntensity;

    FLight_WeatherZone()
    {
        ZoneLocation = FVector::ZeroVector;
        ZoneRadius = 2000.0f;
        WeatherType = EBiomeType::Forest;
        FogDensity = 0.02f;
        FogColor = FLinearColor::White;
        LightIntensity = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_VolumetricSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float HeightFalloff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    FLinearColor InscatteringColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float MaxOpacity;

    FLight_VolumetricSettings()
    {
        FogDensity = 0.02f;
        HeightFalloff = 0.2f;
        InscatteringColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
        MaxOpacity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_WeatherSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_WeatherSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Weather zone management
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void CreateWeatherZone(const FVector& Location, EBiomeType WeatherType, float Radius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateWeatherInZone(const FLight_WeatherZone& WeatherZone);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionBetweenWeatherZones(const FLight_WeatherZone& FromZone, const FLight_WeatherZone& ToZone, float TransitionTime = 5.0f);

    // Volumetric lighting system
    UFUNCTION(BlueprintCallable, Category = "Volumetric Lighting")
    void CreateVolumetricFog(const FVector& Location, const FLight_VolumetricSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Lighting")
    void UpdateVolumetricLighting(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Lighting")
    void SetGlobalVolumetricSettings(const FLight_VolumetricSettings& Settings);

    // Cave lighting enhancement
    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void EnhanceCaveLighting(const FVector& CaveLocation, float Intensity = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CreateFlickeringTorchLight(const FVector& Location, float BaseIntensity = 800.0f);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void UpdateCaveAmbientLighting(float AmbientLevel);

    // Atmospheric persistence system
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FixPersistentAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SaveAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void LoadAtmosphericSettings();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather System")
    TArray<FLight_WeatherZone> ActiveWeatherZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Volumetric Lighting")
    TArray<AActor*> VolumetricFogActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lighting")
    TArray<APointLight*> CaveLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_VolumetricSettings DefaultVolumetricSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float WeatherTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDynamicWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableVolumetricLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableCaveLighting;

private:
    void InitializeWeatherSystem();
    void UpdateDynamicWeather(float DeltaTime);
    void ProcessVolumetricEffects(float DeltaTime);
    void ManageCaveLightingSystem(float DeltaTime);

    float CurrentWeatherTransition;
    bool bIsTransitioning;
};