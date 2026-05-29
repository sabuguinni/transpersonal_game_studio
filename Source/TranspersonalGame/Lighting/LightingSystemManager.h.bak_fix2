#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/VolumetricCloudComponent.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay_B3E : uint8
{
    Dawn = 0,
    Morning,
    Midday,
    Afternoon,
    Dusk,
    Night
};

UENUM(BlueprintType)
enum class ELight_WeatherState_B3E : uint8
{
    Clear = 0,
    PartlyCloudy,
    Overcast,
    Foggy,
    Stormy
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
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
    virtual void Tick(float DeltaTime);

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
    ELight_WeatherState_B3E CurrentWeatherState = ELight_WeatherState_B3E::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    // Lighting presets for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<ELight_TimeOfDay_B3E, FLightingPreset> LightingPresets;

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

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    ELight_TimeOfDay_B3E GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void ApplyAtmosphericCorrection();

    // Weather control functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(ELight_WeatherState_B3E NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherState_B3E TargetWeather, float TransitionTime = 5.0f);

    // Lighting preset functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(ELight_TimeOfDay_B3E TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InterpolateLightingPresets(ELight_TimeOfDay_B3E FromTime, ELight_TimeOfDay_B3E ToTime, float Alpha);

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
    
    // Weather transition variables
    ELight_WeatherState_B3E TargetWeatherState;
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 5.0f;
    bool bIsTransitioningWeather = false;
};