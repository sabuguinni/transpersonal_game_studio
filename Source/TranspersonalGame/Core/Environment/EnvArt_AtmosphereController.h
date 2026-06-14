#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Particles/ParticleSystemComponent.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphereController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsRaining = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoAdvanceTime = true;

    // Atmosphere presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FEnvArt_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FEnvArt_TimeOfDaySettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FEnvArt_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FEnvArt_TimeOfDaySettings NightSettings;

    // Weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FEnvArt_WeatherSettings CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Particle effects for atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    UParticleSystemComponent* DustParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    UParticleSystemComponent* PollenParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    UParticleSystemComponent* RainParticles;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherPreset(const FEnvArt_WeatherSettings& NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateStormyWeather();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateClearWeather();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericEffects();

private:
    // Internal functions
    void UpdateSunPosition();
    void UpdateFogSettings();
    void UpdateParticleEffects();
    void InterpolateTimeOfDaySettings(float Alpha, const FEnvArt_TimeOfDaySettings& A, const FEnvArt_TimeOfDaySettings& B, FEnvArt_TimeOfDaySettings& Result);

    // Cached references to world lighting
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    ASkyLight* SkyLight;

    UPROPERTY()
    UExponentialHeightFogComponent* HeightFog;

    float TimeAccumulator = 0.0f;
};