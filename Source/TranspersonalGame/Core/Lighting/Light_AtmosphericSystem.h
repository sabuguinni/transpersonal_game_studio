#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Light_AtmosphericSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Skies"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudOpacity = 0.8f;

    FLight_AtmosphereSettings()
    {
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
        SkyLightIntensity = 1.0f;
        SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        FogHeightFalloff = 0.2f;
        CloudCoverage = 0.3f;
        CloudOpacity = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class USkyAtmosphereComponent* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class UVolumetricCloudComponent* VolumetricClouds;

    // Time and weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Atmosphere presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_TimeOfDay, FLight_AtmosphereSettings> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_WeatherType, FLight_AtmosphereSettings> WeatherPresets;

    // Current interpolated settings
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FLight_AtmosphereSettings CurrentSettings;

    // Material parameter collection for global lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialParameterCollection* GlobalLightingMPC;

public:
    // Time control functions
    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetDayDuration(float Minutes);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    // Weather control functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherType NewWeather, float TransitionTime = 5.0f);

    // Atmosphere control functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InterpolateAtmosphereSettings(const FLight_AtmosphereSettings& From, const FLight_AtmosphereSettings& To, float Alpha);

    // Preset management
    UFUNCTION(BlueprintCallable, Category = "Presets")
    void InitializeDefaultPresets();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void SaveCurrentAsPreset(ELight_TimeOfDay TimeSlot);

    // Cave and interior lighting support
    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void EnableInteriorMode(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void SetInteriorAmbientLevel(float AmbientMultiplier);

protected:
    // Internal update functions
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateAtmosphere();
    void UpdateMaterialParameters();
    FLight_AtmosphereSettings GetInterpolatedTimeSettings() const;
    FLight_AtmosphereSettings GetWeatherModifiedSettings(const FLight_AtmosphereSettings& BaseSettings) const;

    // Weather transition
    bool bIsTransitioning = false;
    ELight_WeatherType TargetWeather;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 5.0f;

    // Interior lighting state
    bool bInteriorMode = false;
    float InteriorAmbientMultiplier = 0.1f;
};