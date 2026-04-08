#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
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
    Dawn        UMETA(DisplayName = "Dawn (5:00-7:00)"),
    Morning     UMETA(DisplayName = "Morning (7:00-11:00)"),
    Midday      UMETA(DisplayName = "Midday (11:00-15:00)"),
    Afternoon   UMETA(DisplayName = "Afternoon (15:00-18:00)"),
    Dusk        UMETA(DisplayName = "Dusk (18:00-20:00)"),
    Night       UMETA(DisplayName = "Night (20:00-5:00)")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Dense Fog")
};

UENUM(BlueprintType)
enum class EEmotionalTone : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful - Safe moments"),
    Tense       UMETA(DisplayName = "Tense - Something approaches"),
    Danger      UMETA(DisplayName = "Danger - Immediate threat"),
    Mystery     UMETA(DisplayName = "Mystery - Unknown presence"),
    Isolation   UMETA(DisplayName = "Isolation - Alone and vulnerable"),
    Discovery   UMETA(DisplayName = "Discovery - Finding something important")
};

USTRUCT(BlueprintType)
struct FLightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-30.0f, 180.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHaziness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional")
    float TensionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional")
    FLinearColor EmotionalTint = FLinearColor::White;
};

/**
 * Core lighting system that manages dynamic day/night cycle, weather, and emotional atmosphere
 * Based on Lumen GI with Sky Atmosphere, Volumetric Clouds, and advanced fog systems
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULightingSystemCore : public UActorComponent
{
    GENERATED_BODY()

public:
    ULightingSystemCore();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                              FActorComponentTickFunction* ThisTickFunction) override;

    // === CORE LIGHTING CONTROL ===
    
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetTimeOfDay(ETimeOfDay NewTimeOfDay, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetWeatherState(EWeatherState NewWeather, float TransitionDuration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetEmotionalTone(EEmotionalTone NewTone, float TransitionDuration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void EnableDynamicTimeProgression(bool bEnable, float TimeScale = 1.0f);

    // === EMOTIONAL LIGHTING TRIGGERS ===

    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void TriggerDangerLighting(float Intensity = 1.0f, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void TriggerMysteryLighting(float Intensity = 1.0f, float Duration = 8.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void TriggerIsolationLighting(float Intensity = 1.0f, float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void RestoreNeutralLighting(float TransitionDuration = 3.0f);

    // === WEATHER EFFECTS ===

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StartRainEffect(float Intensity = 0.5f, float Duration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StartStormEffect(float Intensity = 0.8f, float Duration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StartFogEffect(float Density = 0.6f, float Duration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void ClearWeatherEffects(float TransitionDuration = 5.0f);

protected:
    // === COMPONENT REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    TObjectPtr<ASkyLight> SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    TObjectPtr<UVolumetricCloudComponent> VolumetricClouds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    TObjectPtr<UExponentialHeightFogComponent> HeightFog;

    // === CURRENT STATE ===

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EWeatherState CurrentWeather = EWeatherState::Clear;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EEmotionalTone CurrentEmotionalTone = EEmotionalTone::Peaceful;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentTimeHour = 8.0f; // 8:00 AM start

    // === LIGHTING PRESETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ETimeOfDay, FLightingPreset> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EWeatherState, FLightingPreset> WeatherPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EEmotionalTone, FLightingPreset> EmotionalPresets;

    // === DYNAMIC TIME SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bDynamicTimeEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeProgressionScale = 1.0f; // 1.0 = real time, 60.0 = 1 minute = 1 hour

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 24.0f; // Full day cycle in real minutes

    // === TRANSITION SYSTEM ===

    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    float TransitionProgress = 0.0f;

    FLightingPreset SourcePreset;
    FLightingPreset TargetPreset;
    float TransitionDuration = 0.0f;
    float TransitionTimer = 0.0f;

private:
    // === INTERNAL METHODS ===

    void InitializeLightingComponents();
    void InitializePresets();
    void UpdateTimeProgression(float DeltaTime);
    void UpdateTransitions(float DeltaTime);
    void ApplyLightingPreset(const FLightingPreset& Preset);
    void StartTransition(const FLightingPreset& Target, float Duration);
    FLightingPreset GetCurrentTargetPreset() const;
    FLightingPreset BlendPresets(const FLightingPreset& A, const FLightingPreset& B, float Alpha) const;
    void FindLightingComponents();
    ETimeOfDay GetTimeOfDayFromHour(float Hour) const;
};