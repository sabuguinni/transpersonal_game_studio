#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "SharedTypes.h"
#include "EnvironmentAtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    // Sun lighting configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Lighting")
    float SunElevationAngle = -30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Lighting")
    float SunAzimuthAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.86f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Lighting")
    float SunIntensity = 3.5f;

    // Fog configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Fog")
    float FogMaxOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);

    // Sky atmosphere settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Atmosphere")
    float RayleighScatteringScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Atmosphere")
    float MieScatteringScale = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Atmosphere")
    float OtherAbsorptionScale = 0.4f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentAtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    // Atmosphere configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings AtmosphereSettings;

    // Time of day control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 14.0f; // 2 PM golden hour

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bEnableDynamicTimeOfDay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDurationMinutes = 20.0f;

    // Biome-specific atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    TMap<EBiomeType, FEnvArt_AtmosphereSettings> BiomeAtmosphereSettings;

    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    ASkyAtmosphere* SkyAtmosphere;

    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiomeAtmosphere(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphereComponents();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Atmosphere")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Atmosphere")
    void CreateVolumetricFog();

protected:
    // Internal methods
    void UpdateSunLighting();
    void UpdateAtmosphericFog();
    void UpdateSkyAtmosphere();
    void CalculateSunPosition(float TimeOfDay, float& OutElevation, float& OutAzimuth);
    FLinearColor CalculateSunColor(float SunElevation);
    float CalculateSunIntensity(float SunElevation);

    // Time of day tracking
    float TimeAccumulator = 0.0f;
    bool bComponentsInitialized = false;

public:
    virtual void Tick(float DeltaTime) override;
};