#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../Audio/Audio_WeatherSystem.h"
#include "VFX_WeatherEffectsManager.generated.h"

// Weather VFX intensity levels
UENUM(BlueprintType)
enum class EVFX_WeatherIntensity : uint8
{
    None        UMETA(DisplayName = "No Weather"),
    Light       UMETA(DisplayName = "Light"),
    Moderate    UMETA(DisplayName = "Moderate"), 
    Heavy       UMETA(DisplayName = "Heavy"),
    Extreme     UMETA(DisplayName = "Extreme")
};

// Weather VFX types that sync with audio system
UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Skies"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Thunderstorm"),
    Wind        UMETA(DisplayName = "Strong Wind"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Dust        UMETA(DisplayName = "Dust Storm")
};

// VFX effect configuration
USTRUCT(BlueprintType)
struct FVFX_WeatherEffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector SpawnScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float IntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoActivate;

    FVFX_WeatherEffectConfig()
    {
        SpawnLocation = FVector::ZeroVector;
        SpawnScale = FVector::OneVector;
        IntensityMultiplier = 1.0f;
        bAutoActivate = true;
    }
};

/**
 * Weather VFX Manager - Synchronizes visual effects with Audio_WeatherSystem
 * Creates realistic prehistoric weather effects: rain, storms, fog, wind, dust
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_WeatherEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_WeatherEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === WEATHER VFX CONTROL ===
    
    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void SetWeatherType(EVFX_WeatherType NewWeatherType, EVFX_WeatherIntensity Intensity = EVFX_WeatherIntensity::Moderate);

    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void TransitionToWeather(EVFX_WeatherType NewWeatherType, EVFX_WeatherIntensity Intensity, float TransitionDuration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void StopAllWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void TriggerLightningStrike(FVector StrikeLocation);

    // === ATMOSPHERIC EFFECTS ===

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void SetAtmosphericScattering(float Density, FLinearColor ScatterColor);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX") 
    void SetVolumetricFog(float Density, float Height, FLinearColor FogColor);

    // === AUDIO SYNC ===

    UFUNCTION(BlueprintCallable, Category = "Audio Sync")
    void SyncWithAudioWeatherSystem(UAudio_WeatherSystem* AudioSystem);

protected:
    // === WEATHER STATE ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    EVFX_WeatherType CurrentWeatherType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    EVFX_WeatherIntensity CurrentIntensity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    bool bIsTransitioning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    float TransitionProgress;

    // === NIAGARA COMPONENTS ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TObjectPtr<UNiagaraComponent> RainEffectComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TObjectPtr<UNiagaraComponent> FogEffectComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TObjectPtr<UNiagaraComponent> LightningEffectComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TObjectPtr<UNiagaraComponent> WindEffectComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TObjectPtr<UNiagaraComponent> DustEffectComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TObjectPtr<UNiagaraComponent> AtmosphericScatteringComponent;

    // === VFX CONFIGURATIONS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configs")
    TMap<EVFX_WeatherType, FVFX_WeatherEffectConfig> WeatherEffectConfigs;

    // === LIGHTNING SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float LightningMinInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float LightningMaxInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float LightningRange;

    FTimerHandle LightningTimerHandle;

    // === AUDIO INTEGRATION ===

    UPROPERTY()
    TWeakObjectPtr<UAudio_WeatherSystem> LinkedAudioSystem;

private:
    // === INTERNAL METHODS ===

    void InitializeNiagaraComponents();
    void UpdateWeatherEffectIntensity(float DeltaTime);
    void HandleWeatherTransition(float DeltaTime);
    void ScheduleNextLightningStrike();
    void ExecuteLightningStrike();
    FVector GetRandomLightningLocation();

    // === TRANSITION DATA ===
    EVFX_WeatherType TargetWeatherType;
    EVFX_WeatherIntensity TargetIntensity;
    float TransitionDuration;
    float TransitionTimer;
};