#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDay
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float Hours = 12.0f; // 0-24 hour format

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float Minutes = 0.0f; // 0-60 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeScale = 1.0f; // Time multiplier for day/night cycle

    FLight_TimeOfDay()
    {
        Hours = 12.0f;
        Minutes = 0.0f;
        TimeScale = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.003996f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    FLight_AtmosphericSettings()
    {
        SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
        SunIntensity = 5.0f;
        RayleighScattering = 0.0331f;
        MieScattering = 0.003996f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (AllowPrivateAccess = "true"))
    FLight_TimeOfDay CurrentTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableDayNightCycle = true;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Atmospheric Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphericSettings AtmosphereSettings;

    // Actor References (auto-found in BeginPlay)
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AVolumetricCloud* VolumetricClouds;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    APostProcessVolume* PostProcessVolume;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hours, float Minutes);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphericCorrection();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void FindAtmosphericActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void FixApocalypticAtmosphere();

protected:
    // Internal Functions
    void UpdateSunPosition();
    void UpdateAtmosphericScattering();
    void UpdateWeatherEffects();
    void UpdateFogSettings();
    void UpdatePostProcessing();

    // Helper Functions
    float CalculateSunAngle() const;
    FLinearColor CalculateSunColor(float SunAngle) const;
    float CalculateSunIntensity(float SunAngle) const;
    void TransitionWeather(float DeltaTime);

private:
    // Internal State
    float WeatherTransitionProgress = 0.0f;
    ELight_WeatherState TargetWeather = ELight_WeatherState::Clear;
    bool bWeatherTransitioning = false;

    // Cached Settings for Smooth Transitions
    FLight_AtmosphericSettings PreviousSettings;
    FLight_AtmosphericSettings TargetSettings;
};