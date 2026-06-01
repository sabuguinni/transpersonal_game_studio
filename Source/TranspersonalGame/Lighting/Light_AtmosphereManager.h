#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/PostProcessVolume.h"
#include "SharedTypes.h"
#include "Light_AtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDay
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float Hour = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float Minute = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeScale = 1.0f;

    FLight_TimeOfDay()
    {
        Hour = 12.0f;
        Minute = 0.0f;
        TimeScale = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeLighting
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor AmbientColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor FogColor = FLinearColor::White;

    FLight_BiomeLighting()
    {
        AmbientColor = FLinearColor::White;
        SunColor = FLinearColor::White;
        SunIntensity = 3.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor::White;
    }
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Storm       UMETA(DisplayName = "Storm")
};

/**
 * Manages dynamic lighting, atmosphere, and weather systems for the prehistoric world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (AllowPrivateAccess = "true"))
    class USkyAtmosphereComponent* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog", meta = (AllowPrivateAccess = "true"))
    class UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clouds", meta = (AllowPrivateAccess = "true"))
    class UVolumetricCloudComponent* VolumetricClouds;

public:
    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    FLight_TimeOfDay CurrentTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 20.0f;

    // Weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherSystem = true;

    // Biome lighting configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TMap<EBiomeType, FLight_BiomeLighting> BiomeLightingConfigs;

    // Lighting functions
    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float Hour, float Minute);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeScale(float NewTimeScale);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherType NewWeather, float TransitionDuration);

    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    void ApplyBiomeLighting(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphericScattering();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateCloudCoverage();

    // Cinematic lighting presets
    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void SetCinematicLighting(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void EnableVolumetricLighting(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void SetGlobalIllumination(bool bUseLumen);

protected:
    // Internal update functions
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherEffects(float DeltaTime);
    void UpdateBiomeSpecificLighting();
    void InitializeLightingComponents();
    void InitializeBiomeLightingConfigs();

    // Lighting calculation helpers
    FRotator CalculateSunRotation(float TimeOfDay) const;
    FLinearColor CalculateSunColor(float TimeOfDay) const;
    float CalculateSunIntensity(float TimeOfDay) const;
    FLinearColor CalculateFogColor(float TimeOfDay, ELight_WeatherType Weather) const;
    float CalculateFogDensity(ELight_WeatherType Weather) const;

private:
    // Internal state
    float WeatherTransitionTimer = 0.0f;
    ELight_WeatherType TargetWeather = ELight_WeatherType::Clear;
    bool bIsTransitioningWeather = false;

    // Cached references
    UPROPERTY()
    APostProcessVolume* GlobalPostProcessVolume;

    // Performance optimization
    float LastUpdateTime = 0.0f;
    const float UpdateInterval = 0.1f; // Update lighting every 0.1 seconds
};