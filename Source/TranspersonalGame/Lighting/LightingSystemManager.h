#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightingSystemManager.generated.h"

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
    Hostile     UMETA(DisplayName = "Hostile")
};

USTRUCT(BlueprintType)
struct FLightingConfiguration
{
    GENERATED_BODY()

    // Sun Light Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator SunRotation = FRotator::ZeroRotator;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunSourceAngle = 0.5f;
    
    // Sky Light Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyColor = FLinearColor(0.2f, 0.4f, 0.8f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkyIntensity = 1.0f;
    
    // Atmosphere Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AtmosphereHaziness = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AtmosphereTint = FLinearColor::White;
    
    // Fog Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogHeightFalloff = 0.2f;
    
    // Cloud Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudCoverage = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudOpacity = 0.8f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CloudWindDirection = FVector(1.0f, 0.0f, 0.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudWindSpeed = 0.1f;
};

/**
 * Central manager for dynamic lighting and atmosphere system
 * Handles day/night cycle, weather transitions, and emotional lighting states
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

    // Core Lighting Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ASkyLight* SkyLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyAtmosphereComponent* SkyAtmosphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UVolumetricCloudComponent* VolumetricClouds;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UExponentialHeightFogComponent* HeightFog;

public:
    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDayHours = 12.0f; // 0-24 hour format
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 20.0f; // Real-time minutes for full day cycle
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoAdvanceTime = true;
    
    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeather = EWeatherState::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState TargetWeather = EWeatherState::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;
    
    // Emotional Tone System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Tone")
    EEmotionalTone CurrentEmotionalTone = EEmotionalTone::Peaceful;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Tone")
    float EmotionalIntensity = 0.5f; // 0-1 range
    
    // Lighting Configurations for different states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<ETimeOfDay, FLightingConfiguration> TimeOfDayConfigurations;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EWeatherState, FLightingConfiguration> WeatherConfigurations;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EEmotionalTone, FLightingConfiguration> EmotionalToneConfigurations;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetTimeOfDay(float Hours);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetWeatherState(EWeatherState NewWeather, float TransitionTime = 5.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetEmotionalTone(EEmotionalTone NewTone, float Intensity = 0.5f);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    ETimeOfDay GetCurrentTimeOfDay() const;
    
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void TriggerLightningStrike(FVector Location, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void CreateAtmosphericTension(float Duration = 10.0f);

private:
    // Internal state
    float WeatherTransitionProgress = 0.0f;
    float EmotionalTransitionProgress = 0.0f;
    bool bIsTransitioningWeather = false;
    
    // Lighting calculation methods
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateEmotionalLighting();
    void ApplyLightingConfiguration(const FLightingConfiguration& Config, float BlendWeight = 1.0f);
    FLightingConfiguration BlendLightingConfigurations(const FLightingConfiguration& A, const FLightingConfiguration& B, float Alpha);
    
    // Component setup
    void InitializeLightingComponents();
    void SetupDefaultConfigurations();
    
    // Atmospheric effects
    void UpdateSunPosition();
    void UpdateSkyAtmosphere();
    void UpdateVolumetricClouds();
    void UpdateHeightFog();
    
    // Emotional lighting effects
    void ApplyTensionLighting();
    void ApplyThreatLighting();
    void ApplyMysteriousLighting();
    void ApplyHostileLighting();
};