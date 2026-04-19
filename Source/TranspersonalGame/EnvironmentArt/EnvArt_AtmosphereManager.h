#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "../SharedTypes.h"
#include "EnvArt_AtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeight = 200.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_VolumetricFogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    FLinearColor VolumetricFogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogExtinctionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float ViewDistance = 6000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core atmosphere components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UDirectionalLightComponent* SunLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkyLightComponent* SkyLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UExponentialHeightFogComponent* FogComponent;

    // Time of day settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FEnvArt_TimeOfDaySettings GoldenHourSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FEnvArt_TimeOfDaySettings MidDaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FEnvArt_TimeOfDaySettings DuskSettings;

    // Volumetric fog settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    FEnvArt_VolumetricFogSettings VolumetricFogSettings;

    // Current time of day (0.0 = dawn, 0.5 = noon, 1.0 = midnight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentTimeOfDay = 0.25f; // Golden hour

    // Day cycle duration in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayCycleDuration = 1200.0f; // 20 minutes

    // Whether to automatically cycle through day/night
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoCycleDayNight = false;

    // Atmosphere control functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetMidDayLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetDuskLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyTimeOfDaySettings(const FEnvArt_TimeOfDaySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableForestAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableVolcanicAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableRiverValleyAtmosphere();

private:
    void UpdateLightingBasedOnTime();
    FEnvArt_TimeOfDaySettings InterpolateTimeOfDaySettings(const FEnvArt_TimeOfDaySettings& SettingsA, const FEnvArt_TimeOfDaySettings& SettingsB, float Alpha);
};