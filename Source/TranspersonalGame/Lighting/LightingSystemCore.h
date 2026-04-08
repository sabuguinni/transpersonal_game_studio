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
    Storm       UMETA(DisplayName = "Thunderstorm")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe - Bright, warm lighting"),
    Cautious    UMETA(DisplayName = "Cautious - Neutral lighting"),
    Dangerous   UMETA(DisplayName = "Dangerous - Dramatic shadows"),
    Predator    UMETA(DisplayName = "Predator - Dark, cold lighting")
};

USTRUCT(BlueprintType)
struct FLightingConfiguration
{
    GENERATED_BODY()

    // Sun Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = 45.0f;

    // Sky Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyIntensity = 1.0f;

    // Fog Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    // Cloud Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudOpacity = 0.8f;

    // Emotional Modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float ThreatMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float WarmthFactor = 1.0f;
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
    // === CORE LIGHTING CONTROL ===
    
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetTimeOfDay(float TimeInHours);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetWeatherState(EWeatherState NewWeatherState, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetThreatLevel(EThreatLevel NewThreatLevel, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void EnableDynamicTimeProgression(bool bEnable, float TimeScale = 1.0f);

    // === LIGHTING QUERIES ===
    
    UFUNCTION(BlueprintPure, Category = "Lighting System")
    ETimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting System")
    EWeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintPure, Category = "Lighting System")
    EThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Lighting System")
    float GetCurrentTimeInHours() const { return CurrentTimeInHours; }

    // === LIGHTING COMPONENTS REFERENCES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class AVolumetricCloud* VolumetricClouds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class AExponentialHeightFog* HeightFog;

private:
    // === INTERNAL STATE ===
    
    UPROPERTY(VisibleAnywhere, Category = "State")
    float CurrentTimeInHours = 12.0f; // Start at midday

    UPROPERTY(VisibleAnywhere, Category = "State")
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Midday;

    UPROPERTY(VisibleAnywhere, Category = "State")
    EWeatherState CurrentWeatherState = EWeatherState::PartlyCloudy;

    UPROPERTY(VisibleAnywhere, Category = "State")
    EThreatLevel CurrentThreatLevel = EThreatLevel::Safe;

    UPROPERTY(EditAnywhere, Category = "Settings")
    bool bDynamicTimeProgression = true;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float TimeProgressionScale = 1.0f; // 1.0 = real time, 60.0 = 1 minute = 1 hour

    // === LIGHTING CONFIGURATIONS ===
    
    UPROPERTY(EditAnywhere, Category = "Lighting Presets")
    TMap<ETimeOfDay, FLightingConfiguration> TimeOfDayConfigurations;

    UPROPERTY(EditAnywhere, Category = "Lighting Presets")
    TMap<EWeatherState, FLightingConfiguration> WeatherConfigurations;

    UPROPERTY(EditAnywhere, Category = "Lighting Presets")
    TMap<EThreatLevel, FLightingConfiguration> ThreatLevelConfigurations;

    // === INTERNAL METHODS ===
    
    void UpdateTimeOfDay();
    void UpdateLightingConfiguration();
    void ApplyLightingConfiguration(const FLightingConfiguration& Config, float BlendWeight = 1.0f);
    void InitializeLightingComponents();
    void SetupDefaultConfigurations();
    FLightingConfiguration BlendConfigurations(const FLightingConfiguration& A, const FLightingConfiguration& B, float Alpha);
    
    // === TRANSITION SYSTEM ===
    
    struct FLightingTransition
    {
        FLightingConfiguration StartConfig;
        FLightingConfiguration TargetConfig;
        float TransitionTime;
        float CurrentTime;
        bool bActive;
    };

    FLightingTransition CurrentTransition;
    void UpdateTransition(float DeltaTime);
};