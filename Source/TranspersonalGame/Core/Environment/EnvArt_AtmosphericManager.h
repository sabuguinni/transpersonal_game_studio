#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/World.h"
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
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Humid       UMETA(DisplayName = "Humid")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = -30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAzimuth = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Temperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricScattering = 2.0f;
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
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float DirectionalInscatteringExponent = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor DirectionalInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricScatteringDistribution = 0.2f;
};

/**
 * Atmospheric Manager for creating and controlling Cretaceous period environmental atmosphere
 * Manages lighting, fog, weather effects, and day/night cycles for prehistoric immersion
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core atmospheric components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Current atmospheric state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 8.0f;

    // Weather transition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.1", ClampMax = "60.0"))
    float WeatherTransitionTime = 10.0f;

    // Atmospheric functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherState(EEnvArt_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLightingSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateSkyAtmosphere();

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor")
    void PreviewGoldenHour();

    UFUNCTION(CallInEditor, Category = "Editor")
    void PreviewStormyWeather();

    UFUNCTION(CallInEditor, Category = "Editor")
    void ResetToDefault();

protected:
    // Internal references
    UPROPERTY()
    class ADirectionalLight* DirectionalLightActor;

    UPROPERTY()
    class AExponentialHeightFog* FogActor;

    UPROPERTY()
    class ASkyAtmosphere* SkyAtmosphereActor;

    // Internal state
    float WeatherTransitionTimer = 0.0f;
    bool bIsTransitioning = false;
    EEnvArt_WeatherState TargetWeather;

    // Internal functions
    void FindOrCreateLightingActors();
    void UpdateTimeBasedLighting();
    void UpdateWeatherEffects();
    FEnvArt_LightingSettings GetLightingForTime(EEnvArt_TimeOfDay TimeOfDay) const;
    FEnvArt_FogSettings GetFogForWeather(EEnvArt_WeatherState Weather) const;
};