#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn (5-7 AM)"),
    Morning     UMETA(DisplayName = "Morning (7-11 AM)"),
    Midday      UMETA(DisplayName = "Midday (11 AM-2 PM)"),
    Afternoon   UMETA(DisplayName = "Afternoon (2-6 PM)"),
    Dusk        UMETA(DisplayName = "Dusk (6-8 PM)"),
    Night       UMETA(DisplayName = "Night (8 PM-5 AM)")
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
    Peaceful    UMETA(DisplayName = "Peaceful - Safe exploration"),
    Tense       UMETA(DisplayName = "Tense - Potential danger"),
    Threatening UMETA(DisplayName = "Threatening - Predator nearby"),
    Panic       UMETA(DisplayName = "Panic - Active chase"),
    Wonder      UMETA(DisplayName = "Wonder - Discovery moment"),
    Isolation   UMETA(DisplayName = "Isolation - Alone in vastness")
};

USTRUCT(BlueprintType)
struct FAtmosphereSettings
{
    GENERATED_BODY()

    // Sun/Moon positioning
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    float SunAngle = 45.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    float SunIntensity = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    FLinearColor SunColor = FLinearColor::White;
    
    // Sky Atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHeight = 60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 1.0f;
    
    // Fog settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    
    // Cloud coverage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudOpacity = 0.8f;
};

/**
 * Central manager for all atmospheric and lighting systems
 * Handles day/night cycle, weather transitions, and emotional lighting states
 */
UCLASS()
class TRANSPERSONALGAME_API AAtmosphereManager : public AActor
{
    GENERATED_BODY()
    
public:    
    AAtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* MoonLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ASkyLight* SkyLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    USkyAtmosphereComponent* SkyAtmosphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    UVolumetricCloudComponent* VolumetricClouds;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    UExponentialHeightFogComponent* HeightFog;

public:
    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDayHours = 12.0f; // 0-24 hour format
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoAdvanceTime = true;
    
    // Weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeather = EWeatherState::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState TargetWeather = EWeatherState::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;
    
    // Emotional lighting system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Lighting")
    EEmotionalTone CurrentEmotionalTone = EEmotionalTone::Peaceful;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Lighting")
    float EmotionalIntensity = 1.0f; // 0-1 blend factor
    
    // Atmosphere presets for different times/weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ETimeOfDay, FAtmosphereSettings> TimeOfDayPresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EWeatherState, FAtmosphereSettings> WeatherPresets;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EEmotionalTone, FAtmosphereSettings> EmotionalPresets;

    // Public interface functions
    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float Hours);
    
    UFUNCTION(BlueprintCallable, Category = "Time System")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;
    
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(EWeatherState NewWeather, float TransitionDuration = 5.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void SetEmotionalTone(EEmotionalTone NewTone, float Intensity = 1.0f, float TransitionDuration = 2.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FAtmosphereSettings& Settings, float BlendWeight = 1.0f);
    
    // Event dispatchers for other systems
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, ETimeOfDay, NewTimeOfDay);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTimeOfDayChanged OnTimeOfDayChanged;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, EWeatherState, NewWeather);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWeatherChanged OnWeatherChanged;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalToneChanged, EEmotionalTone, NewTone, float, Intensity);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEmotionalToneChanged OnEmotionalToneChanged;

private:
    // Internal state tracking
    ETimeOfDay LastTimeOfDay;
    float WeatherBlendAlpha = 0.0f;
    float EmotionalBlendAlpha = 0.0f;
    float EmotionalTransitionDuration = 2.0f;
    float EmotionalTransitionTimer = 0.0f;
    
    // Internal update functions
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateEmotionalLighting(float DeltaTime);
    void UpdateSunPosition();
    void UpdateMoonPosition();
    void BlendAtmosphereSettings();
    
    // Initialization
    void InitializeComponents();
    void SetupDefaultPresets();
    void FindOrCreateLightingActors();
};