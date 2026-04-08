#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
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
enum class EMoodState : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful"),
    Tense       UMETA(DisplayName = "Tense"),
    Threatening UMETA(DisplayName = "Threatening"),
    Dangerous   UMETA(DisplayName = "Dangerous"),
    Terror      UMETA(DisplayName = "Terror")
};

USTRUCT(BlueprintType)
struct FLightingPreset
{
    GENERATED_BODY()

    // Sun/Moon Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunAngle = 45.0f;
    
    // Sky Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkyIntensity = 1.0f;
    
    // Fog Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogHeightFalloff = 0.2f;
    
    // Cloud Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudCoverage = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudOpacity = 0.8f;
    
    // Post Process
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Exposure = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ColorGrading = FLinearColor::White;
};

/**
 * Central manager for all lighting and atmospheric systems
 * Handles day/night cycle, weather, mood-based lighting transitions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class UDirectionalLightComponent* SunLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class UDirectionalLightComponent* MoonLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class USkyLightComponent* SkyLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class USkyAtmosphereComponent* SkyAtmosphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class UVolumetricCloudComponent* VolumetricClouds;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Core")
    class UExponentialHeightFogComponent* HeightFog;

public:
    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDayHours = 12.0f; // 0-24 hours
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoAdvanceTime = true;
    
    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeather = EWeatherState::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bDynamicWeather = true;
    
    // Mood System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    EMoodState CurrentMood = EMoodState::Peaceful;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood")
    float MoodTransitionSpeed = 2.0f;
    
    // Lighting Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ETimeOfDay, FLightingPreset> TimePresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EWeatherState, FLightingPreset> WeatherPresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EMoodState, FLightingPreset> MoodPresets;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float Hours);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetWeatherState(EWeatherState NewWeather);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetMoodState(EMoodState NewMood);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void TransitionToPreset(const FLightingPreset& TargetPreset, float TransitionTime = 2.0f);
    
    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    ETimeOfDay GetCurrentTimeOfDay() const;
    
    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    float GetSunAngle() const;
    
    UFUNCTION(BlueprintPure, Category = "Lighting Info")
    bool IsNightTime() const;

private:
    void UpdateLighting(float DeltaTime);
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeather(float DeltaTime);
    void UpdateMood(float DeltaTime);
    
    void ApplyLightingPreset(const FLightingPreset& Preset);
    FLightingPreset BlendPresets(const FLightingPreset& A, const FLightingPreset& B, float Alpha);
    
    void InitializeDefaultPresets();
    void SetupLumenSettings();
    void SetupVirtualShadowMaps();
    
    // Current blend states
    FLightingPreset CurrentPreset;
    FLightingPreset TargetPreset;
    float BlendAlpha = 0.0f;
    float BlendSpeed = 1.0f;
    
    // Weather transition
    EWeatherState TargetWeather;
    float WeatherBlendAlpha = 0.0f;
    
    // Mood transition
    EMoodState TargetMood;
    float MoodBlendAlpha = 0.0f;
};