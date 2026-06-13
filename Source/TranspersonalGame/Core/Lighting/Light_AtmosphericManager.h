#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "SharedTypes.h"
#include "Light_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.71f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FRotator SunRotation = FRotator(-45.0f, 30.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SkyLightIntensity = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SkyLightColor = FLinearColor(0.78f, 0.86f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor FogColor = FLinearColor(0.71f, 0.63f, 0.47f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_FireLightSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float Intensity = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    FLinearColor Color = FLinearColor(1.0f, 0.59f, 0.20f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float AttenuationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float Temperature = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    bool bFlickerEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FlickerIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FlickerSpeed = 2.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric Lighting")
    class ASkyLight* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric Lighting")
    class AExponentialHeightFog* AtmosphericFog;

    // Time of day settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Configuration")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Configuration")
    FLight_TimeOfDaySettings DaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Configuration")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Configuration")
    FLight_TimeOfDaySettings NightSettings;

    // Fire lighting settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    FLight_FireLightSettings FireLightSettings;

    // Dynamic time control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Control")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Control")
    float TimeSpeed = 1.0f; // Speed multiplier for time progression

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Control")
    bool bEnableTimeProgression = true;

    // Cretaceous period specific settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    bool bEnableCretaceousAtmosphere = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float CretaceousHumidity = 0.8f; // High humidity for period

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float CretaceousTemperature = 28.0f; // Average temperature in Celsius

    // Fire light management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Lighting")
    TArray<class APointLight*> FireLights;

public:
    // Core atmospheric functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void EnableCretaceousAtmosphere(bool bEnable);

    // Fire lighting functions
    UFUNCTION(BlueprintCallable, Category = "Fire Lighting")
    class APointLight* CreateFireLight(FVector Location, FLight_FireLightSettings Settings);

    UFUNCTION(BlueprintCallable, Category = "Fire Lighting")
    void UpdateFireLights();

    UFUNCTION(BlueprintCallable, Category = "Fire Lighting")
    void SetFireFlicker(bool bEnable);

    // Lumen configuration
    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void EnableLumenGlobalIllumination(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void ConfigureLumenSettings();

    // Weather effects
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void CreateVolumetricClouds();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FLight_TimeOfDaySettings GetCurrentTimeSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SaveAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void LoadAtmosphericSettings();

private:
    // Internal update functions
    void UpdateSunLighting(const FLight_TimeOfDaySettings& Settings);
    void UpdateSkyLighting(const FLight_TimeOfDaySettings& Settings);
    void UpdateFogSettings(const FLight_TimeOfDaySettings& Settings);
    void UpdateFireFlicker(float DeltaTime);
    FLight_TimeOfDaySettings InterpolateTimeSettings(float TimeOfDay) const;

    // Fire flicker variables
    float FireFlickerTime = 0.0f;
    TArray<float> FireBaseIntensities;
};