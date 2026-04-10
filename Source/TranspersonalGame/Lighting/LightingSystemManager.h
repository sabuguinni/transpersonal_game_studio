#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/VolumetricCloud.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn = 0,
    Morning,
    Noon,
    Afternoon,
    Dusk,
    Night
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear = 0,
    PartlyCloudy,
    Overcast,
    Foggy,
    Stormy
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator SunRotation = FRotator(-30.0f, 180.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AtmosphericPerspective = 1.0f;
};

/**
 * Manages dynamic lighting, day/night cycles, and atmospheric effects for the prehistoric world
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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting System")
    class USceneComponent* RootSceneComponent;

    // Sun and sky system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    class AVolumetricCloud* VolumetricClouds;

    // Time and weather control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayNightCycleSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeatherState = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherTransitionSpeed = 0.5f;

    // Lighting presets for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<ETimeOfDay, FLightingPreset> LightingPresets;

    // Interior lighting settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    float CaveLightIntensity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    FLinearColor CaveLightColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    float CaveLightRadius = 800.0f;

public:
    // Day/Night cycle functions
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetDayNightCycleSpeed(float NewSpeed);

    // Weather control functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(EWeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(EWeatherState TargetWeather, float TransitionTime = 5.0f);

    // Lighting preset functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(ETimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InterpolateLightingPresets(ETimeOfDay FromTime, ETimeOfDay ToTime, float Alpha);

    // Interior lighting functions
    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    class APointLight* CreateCaveLight(FVector Location, float Intensity = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    class ASpotLight* CreateCaveEntranceLight(FVector Location, FRotator Rotation);

    // Atmospheric effects
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void UpdateAtmosphericFog(float Density, FLinearColor Color, float HeightFalloff);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void UpdateVolumetricClouds(float Coverage, float Density);

    // Lumen configuration
    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void ConfigureLumenSettings(bool bEnableGI = true, bool bEnableReflections = true);

private:
    void InitializeLightingSystem();
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void UpdateSunPosition();
    void InitializeLightingPresets();
    
    // Weather transition variables
    EWeatherState TargetWeatherState;
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 5.0f;
    bool bIsTransitioningWeather = false;
};