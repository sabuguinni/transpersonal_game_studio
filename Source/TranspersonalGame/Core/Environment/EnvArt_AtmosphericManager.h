#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Sound/AudioVolume.h"
#include "TranspersonalGame.h"
#include "EnvArt_AtmosphericManager.generated.h"

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
    FRotator SunRotation = FRotator(-45.0f, 225.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bCastVolumetricShadows = true;

    FEnvArt_LightingSettings()
    {
        SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
        SunIntensity = 3.5f;
        SunRotation = FRotator(-45.0f, 225.0f, 0.0f);
        bCastVolumetricShadows = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    FEnvArt_FogSettings()
    {
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        bVolumetricFog = true;
        VolumetricFogScatteringDistribution = 0.2f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_AtmosphericManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current atmospheric state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_WeatherState CurrentWeather = EEnvArt_WeatherState::Clear;

    // Lighting configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FEnvArt_LightingSettings LightingSettings;

    // Fog configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FEnvArt_FogSettings FogSettings;

    // Time progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TimeProgressionSpeed = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Time")
    float CurrentDayTime = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    // Weather transition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "1.0", ClampMax = "300.0"))
    float WeatherTransitionDuration = 30.0f;

    // Atmospheric functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherState(EEnvArt_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere", CallInEditor = true)
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere", CallInEditor = true)
    void CreateAtmosphericParticles();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SpawnAmbientAudioVolumes();

private:
    // Internal references
    UPROPERTY()
    ADirectionalLight* DirectionalLightActor;

    UPROPERTY()
    AExponentialHeightFog* FogActor;

    // Internal state
    float WeatherTransitionTimer = 0.0f;
    EEnvArt_WeatherState TargetWeatherState;
    bool bWeatherTransitioning = false;

    // Helper functions
    void FindAtmosphericActors();
    void UpdateTimeBasedLighting(float DeltaTime);
    void ProcessWeatherTransition(float DeltaTime);
    FLinearColor CalculateSunColorForTime(float TimeOfDay);
    float CalculateSunIntensityForTime(float TimeOfDay);
    FRotator CalculateSunRotationForTime(float TimeOfDay);
};