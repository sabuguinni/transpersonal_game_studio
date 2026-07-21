#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "SharedTypes.h"
#include "World_WeatherSystemManager.generated.h"

// Weather state enumeration for prehistoric environments
UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    PartlyCloudy    UMETA(DisplayName = "Partly Cloudy"),
    Overcast        UMETA(DisplayName = "Overcast"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm    UMETA(DisplayName = "Thunderstorm"),
    Fog             UMETA(DisplayName = "Fog"),
    Sandstorm       UMETA(DisplayName = "Sandstorm"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    Blizzard        UMETA(DisplayName = "Blizzard")
};

// Weather transition data
USTRUCT(BlueprintType)
struct FWorld_WeatherTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Probability;

    FWorld_WeatherTransition()
    {
        FromState = EWorld_WeatherState::Clear;
        ToState = EWorld_WeatherState::Clear;
        TransitionDuration = 60.0f;
        Probability = 0.1f;
    }
};

// Weather audio configuration
USTRUCT(BlueprintType)
struct FWorld_WeatherAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> IntensitySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime;

    FWorld_WeatherAudioConfig()
    {
        BaseVolume = 0.3f;
        MaxVolume = 0.8f;
        FadeInTime = 5.0f;
        FadeOutTime = 3.0f;
    }
};

// Weather visual effects configuration
USTRUCT(BlueprintType)
struct FWorld_WeatherVFXConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UParticleSystem> ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float SkyLightIntensity;

    FWorld_WeatherVFXConfig()
    {
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        FogDensity = 0.02f;
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SkyLightIntensity = 1.0f;
    }
};

/**
 * Weather System Manager for prehistoric environments
 * Handles dynamic weather transitions, audio, and visual effects
 * Integrates with biome systems and day/night cycles
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_WeatherSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_WeatherSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather state management
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(EWorld_WeatherState NewState, bool bForceImmediate = false);

    UFUNCTION(BlueprintPure, Category = "Weather")
    EWorld_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    EWorld_WeatherState GetTargetWeatherState() const { return TargetWeatherState; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    float GetWeatherIntensity() const { return CurrentWeatherIntensity; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    bool IsTransitioning() const { return bIsTransitioning; }

    // Weather system control
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartWeatherSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StopWeatherSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void PauseWeatherSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ResumeWeatherSystem();

    // Weather configuration
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherTransitionTime(float NewTransitionTime);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherChangeFrequency(float NewFrequency);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetBiomeInfluence(EBiomeType BiomeType, float Influence);

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateWeatherAudio(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetWeatherAudioVolume(float Volume);

    // VFX management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateWeatherVFX(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetWeatherParticleIntensity(float Intensity);

    // Event callbacks
    UFUNCTION(BlueprintImplementableEvent, Category = "Weather")
    void OnWeatherStateChanged(EWorld_WeatherState OldState, EWorld_WeatherState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather")
    void OnWeatherTransitionStarted(EWorld_WeatherState FromState, EWorld_WeatherState ToState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather")
    void OnWeatherTransitionCompleted(EWorld_WeatherState NewState);

protected:
    // Core weather state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    EWorld_WeatherState CurrentWeatherState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    EWorld_WeatherState TargetWeatherState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    float CurrentWeatherIntensity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    bool bIsTransitioning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    float TransitionProgress;

    // Weather system configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Config")
    float WeatherTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Config")
    float WeatherChangeFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Config")
    float MinWeatherDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Config")
    float MaxWeatherDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Config")
    bool bEnableAutomaticWeatherChanges;

    // Weather transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Transitions")
    TArray<FWorld_WeatherTransition> WeatherTransitions;

    // Audio configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<EWorld_WeatherState, FWorld_WeatherAudioConfig> WeatherAudioConfigs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> IntensityAudioComponent;

    // VFX configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<EWorld_WeatherState, FWorld_WeatherVFXConfig> WeatherVFXConfigs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UParticleSystemComponent> WeatherParticleComponent;

    // Lighting references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ASkyLight> SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AExponentialHeightFog> HeightFog;

    // Timing and state management
    float WeatherStateTimer;
    float NextWeatherChangeTime;
    float TransitionStartTime;

    // Biome influence factors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    TMap<EBiomeType, float> BiomeWeatherInfluence;

private:
    // Internal weather management
    void UpdateWeatherTransition(float DeltaTime);
    void ProcessAutomaticWeatherChanges(float DeltaTime);
    EWorld_WeatherState SelectNextWeatherState();
    float CalculateWeatherTransitionProbability(EWorld_WeatherState FromState, EWorld_WeatherState ToState);
    
    // Component management
    void InitializeAudioComponents();
    void InitializeVFXComponents();
    void FindLightingActors();
    
    // Weather effects
    void ApplyWeatherEffects(EWorld_WeatherState WeatherState, float Intensity);
    void InterpolateWeatherEffects(EWorld_WeatherState FromState, EWorld_WeatherState ToState, float Alpha);
    
    // Utility functions
    void LoadWeatherConfigurations();
    void SetupDefaultWeatherTransitions();
    void ValidateWeatherSystem();
};