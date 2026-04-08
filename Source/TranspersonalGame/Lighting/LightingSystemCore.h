#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "LightingSystemCore.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn (5:00-7:00)"),
    Morning     UMETA(DisplayName = "Morning (7:00-11:00)"),
    Midday      UMETA(DisplayName = "Midday (11:00-14:00)"),
    Afternoon   UMETA(DisplayName = "Afternoon (14:00-17:00)"),
    Dusk        UMETA(DisplayName = "Dusk (17:00-19:00)"),
    Night       UMETA(DisplayName = "Night (19:00-5:00)")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Dense Fog")
};

UENUM(BlueprintType)
enum class EEmotionalTone : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful - Safe moments"),
    Tense       UMETA(DisplayName = "Tense - Danger nearby"),
    Terrifying  UMETA(DisplayName = "Terrifying - Immediate threat"),
    Mysterious  UMETA(DisplayName = "Mysterious - Unknown presence"),
    Melancholy  UMETA(DisplayName = "Melancholy - Loss/sadness"),
    Wonder      UMETA(DisplayName = "Wonder - Discovery moments")
};

USTRUCT(BlueprintType)
struct FLightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHaziness = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Contrast = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Saturation = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    FLinearColor ColorGrading = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
};

UCLASS()
class TRANSPERSONALGAME_API ALightingSystemCore : public AActor
{
    GENERATED_BODY()

public:
    ALightingSystemCore();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Lighting Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class USkyAtmosphereComponent* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class UVolumetricCloudComponent* VolumetricClouds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PostProcess")
    class UPostProcessComponent* PostProcessComponent;

public:
    // Time and Weather Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationInMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeather = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional")
    EEmotionalTone CurrentEmotionalTone = EEmotionalTone::Tense;

    // Lighting Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ETimeOfDay, FLightingPreset> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EWeatherState, FLightingPreset> WeatherPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EEmotionalTone, FLightingPreset> EmotionalPresets;

    // Dynamic Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
    bool bEnableTimeProgression = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
    bool bEnableWeatherChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
    float TransitionSpeed = 1.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(EWeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetEmotionalTone(EEmotionalTone NewTone);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToPreset(const FLightingPreset& TargetPreset, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetDangerLighting(bool bIsDangerous);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSafeLighting();

private:
    void UpdateLighting();
    void UpdateSunPosition();
    void UpdateAtmosphere();
    void UpdateFog();
    void UpdatePostProcess();
    void ApplyEmotionalTone();
    
    FLightingPreset GetCurrentPreset() const;
    FLightingPreset BlendPresets(const FLightingPreset& A, const FLightingPreset& B, float Alpha) const;

    // Internal state
    FLightingPreset CurrentPreset;
    FLightingPreset TargetPreset;
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 2.0f;
};