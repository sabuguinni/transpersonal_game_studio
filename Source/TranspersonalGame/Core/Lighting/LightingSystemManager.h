#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Sunset      UMETA(DisplayName = "Sunset"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Mist        UMETA(DisplayName = "Mist")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor RayleighScattering = FLinearColor(0.005802f, 0.013558f, 0.033100f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 0.003996f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor MieScattering = FLinearColor(0.003996f, 0.003996f, 0.003996f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieAbsorptionScale = 0.000444f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor MieAbsorption = FLinearColor(0.000444f, 0.000444f, 0.000444f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float OzoneAbsorptionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor OzoneAbsorption = FLinearColor(0.000650f, 0.001881f, 0.000085f, 1.0f);

    FLight_AtmosphereSettings()
    {
        // Cretaceous period atmosphere defaults - clear tropical blue sky
    }
};

USTRUCT(BlueprintType)
struct FLight_SunSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Intensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor Color = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Temperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator Rotation = FRotator(-45.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    bool bCastShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    bool bCastVolumetricShadow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float VolumetricScatteringIntensity = 1.0f;

    FLight_SunSettings()
    {
        // Warm amber sunlight for Cretaceous period
    }
};

USTRUCT(BlueprintType)
struct FLight_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float StartDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogCutoffDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.447f, 0.639f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor DirectionalInscatteringColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float DirectionalInscatteringExponent = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float DirectionalInscatteringStartDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor VolumetricFogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor VolumetricFogEmissive = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogExtinctionScale = 1.0f;

    FLight_FogSettings()
    {
        // Subtle atmospheric haze for prehistoric environment
    }
};

USTRUCT(BlueprintType)
struct FLight_PostProcessSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Exposure = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Contrast = 1.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Saturation = 1.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Gamma = 2.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Shadows = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Highlights = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float WhiteTemp = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float WhiteTint = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    bool bEnableBloom = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float BloomIntensity = 0.675f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float BloomThreshold = -1.0f;

    FLight_PostProcessSettings()
    {
        // Cinematic color grading for prehistoric atmosphere
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // === TIME OF DAY SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bEnableTimeOfDayProgression = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    ELight_TimeOfDay CurrentTimePhase = ELight_TimeOfDay::Midday;

    // === WEATHER SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity = 1.0f;

    // === LIGHTING SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FLight_SunSettings SunSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FLight_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FLight_FogSettings FogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FLight_PostProcessSettings PostProcessSettings;

    // === REFERENCES TO WORLD ACTORS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World References")
    class AVolumetricCloud* VolumetricClouds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World References")
    class APostProcessVolume* PostProcessVolume;

    // === BLUEPRINT FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetWeather(ELight_WeatherType NewWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void ApplyAtmosphericCorrection();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void UpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void UpdateFog();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void UpdatePostProcess();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void FindWorldLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    FLinearColor GetSunColorForTime(float TimeHours);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    float GetSunIntensityForTime(float TimeHours);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    FRotator GetSunRotationForTime(float TimeHours);

    UFUNCTION(BlueprintPure, Category = "Lighting System")
    ELight_TimeOfDay GetCurrentTimePhase() const { return CurrentTimePhase; }

    UFUNCTION(BlueprintPure, Category = "Lighting System")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting System")
    ELight_WeatherType GetCurrentWeather() const { return CurrentWeather; }

private:
    // === INTERNAL FUNCTIONS ===
    void UpdateTimePhase();
    void InterpolateLightingSettings(float DeltaTime);
    void ApplyCretaceousAtmosphere();
    
    // === TRANSITION VARIABLES ===
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 5.0f;
    ELight_WeatherType TargetWeather = ELight_WeatherType::Clear;
    
    // === CACHED SETTINGS FOR INTERPOLATION ===
    FLight_SunSettings PreviousSunSettings;
    FLight_AtmosphereSettings PreviousAtmosphereSettings;
    FLight_FogSettings PreviousFogSettings;
    FLight_PostProcessSettings PreviousPostProcessSettings;
};

#include "LightingSystemManager.generated.h"