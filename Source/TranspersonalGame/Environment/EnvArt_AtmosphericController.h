#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/AmbientSound.h"
#include "EnvArt_AtmosphericController.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricFogDensity = 0.02f;

    FEnvArt_LightingSettings()
    {
        SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
        SunIntensity = 3.5f;
        SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
        SkyLightColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
        SkyLightIntensity = 1.0f;
        VolumetricFogDensity = 0.02f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float ParticleDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    FEnvArt_AtmosphericSettings()
    {
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        ParticleDensity = 1.0f;
        WindStrength = 0.5f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericController : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EEnvArt_WeatherState CurrentWeatherState = EEnvArt_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_LightingSettings LightingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeProgressionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableTimeProgression = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionDuration = 30.0f;

private:
    UPROPERTY()
    ADirectionalLight* DirectionalLightActor;

    UPROPERTY()
    ASkyLight* SkyLightActor;

    UPROPERTY()
    TArray<UParticleSystemComponent*> AtmosphericParticles;

    UPROPERTY()
    TArray<UAudioComponent*> AmbientSoundComponents;

    float CurrentTimeOfDayFloat = 14.0f; // 2 PM start
    float WeatherTransitionTimer = 0.0f;
    EEnvArt_WeatherState TargetWeatherState = EEnvArt_WeatherState::Clear;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeatherState(EEnvArt_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void TransitionToWeather(EEnvArt_WeatherState NewWeatherState, float TransitionDuration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateLightingForTimeOfDay();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SpawnAtmosphericParticles(const FVector& Location, float Density = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void CreateAmbientSoundZone(const FVector& Location, class USoundBase* AmbientSound, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintPure, Category = "Atmospheric Control")
    float GetCurrentTimeOfDayFloat() const { return CurrentTimeOfDayFloat; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric Control")
    EEnvArt_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric Control")
    EEnvArt_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

private:
    void FindLightingActors();
    void UpdateTimeProgression(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    FEnvArt_LightingSettings GetLightingSettingsForTime(EEnvArt_TimeOfDay TimeOfDay);
    FEnvArt_AtmosphericSettings GetAtmosphericSettingsForWeather(EEnvArt_WeatherState WeatherState);
    void ApplyLightingSettings(const FEnvArt_LightingSettings& Settings);
    void ApplyAtmosphericSettings(const FEnvArt_AtmosphericSettings& Settings);
};