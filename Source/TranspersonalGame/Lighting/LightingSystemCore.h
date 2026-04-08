#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightingSystemCore.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    DeepNight   UMETA(DisplayName = "Deep Night")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

UENUM(BlueprintType)
enum class EEmotionalTone : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful"),
    Tense       UMETA(DisplayName = "Tense"),
    Threatening UMETA(DisplayName = "Threatening"),
    Mysterious  UMETA(DisplayName = "Mysterious"),
    Dangerous   UMETA(DisplayName = "Dangerous")
};

USTRUCT(BlueprintType)
struct FLightingMood
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    float ShadowIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    float ContrastBoost = 1.2f;
};

/**
 * Core lighting system that manages dynamic day/night cycle, weather, and emotional lighting
 * Based on Lumen GI with Sky Atmosphere and Volumetric Clouds
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULightingSystemCore : public UActorComponent
{
    GENERATED_BODY()

public:
    ULightingSystemCore();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TIME OF DAY SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeHours = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoAdvanceTime = true;

    // === WEATHER SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeather = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bDynamicWeather = true;

    // === EMOTIONAL LIGHTING ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Lighting")
    EEmotionalTone CurrentEmotionalTone = EEmotionalTone::Peaceful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Lighting")
    float EmotionalIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Lighting")
    TMap<EEmotionalTone, FLightingMood> EmotionalMoods;

    // === COMPONENT REFERENCES ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UVolumetricCloudComponent> VolumetricClouds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkyLightComponent> SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UExponentialHeightFogComponent> HeightFog;

    // === PUBLIC FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetWeather(EWeatherState NewWeather, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetEmotionalTone(EEmotionalTone NewTone, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void TriggerLightningStrike(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetDinosaurProximityLighting(bool bNearDangerous, float Intensity = 1.0f);

private:
    // === INTERNAL STATE ===
    FLightingMood CurrentMood;
    FLightingMood TargetMood;
    float MoodTransitionProgress = 0.0f;
    float MoodTransitionDuration = 2.0f;
    bool bTransitioning = false;

    // Weather transition
    FLightingMood WeatherTargetMood;
    float WeatherTransitionProgress = 0.0f;
    float WeatherTransitionDuration = 5.0f;
    bool bWeatherTransitioning = false;

    // Lightning system
    float LastLightningTime = 0.0f;
    float LightningCooldown = 10.0f;

    // === INTERNAL FUNCTIONS ===
    void InitializeComponents();
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateEmotionalTransition(float DeltaTime);
    void ApplyLightingMood(const FLightingMood& Mood);
    void CalculateTimeBasedMood();
    void CalculateWeatherMood();
    FLightingMood BlendMoods(const FLightingMood& MoodA, const FLightingMood& MoodB, float Alpha);
    void UpdateSunPosition();
    void UpdateAtmosphereProperties();
    void UpdateCloudProperties();
    void UpdateFogProperties();
    void InitializeEmotionalMoods();
};