#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "VFX_CretaceousEnvironmentController.generated.h"

UENUM(BlueprintType)
enum class EVFX_CretaceousWeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear Sky"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Fog             UMETA(DisplayName = "Dense Fog"),
    WindStorm       UMETA(DisplayName = "Wind Storm"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash")
};

UENUM(BlueprintType)
enum class EVFX_CretaceousTimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Midday          UMETA(DisplayName = "Midday"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FVFX_CretaceousAtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.8f, 0.85f, 0.9f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float HumidityLevel = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float TemperatureLevel = 0.85f;
};

USTRUCT(BlueprintType)
struct FVFX_CretaceousParticleEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    float LifeTime = 10.0f;
};

/**
 * VFX Controller for realistic Cretaceous period environmental effects
 * Handles weather, atmospheric conditions, and environmental particle systems
 * Based on scientific understanding of late Cretaceous climate (66-100 million years ago)
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CretaceousEnvironmentController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CretaceousEnvironmentController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === ATMOSPHERIC CONTROL ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Environment")
    EVFX_CretaceousWeatherType CurrentWeather = EVFX_CretaceousWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Environment")
    EVFX_CretaceousTimeOfDay CurrentTimeOfDay = EVFX_CretaceousTimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Environment")
    FVFX_CretaceousAtmosphereSettings AtmosphereSettings;

    // === PARTICLE SYSTEMS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TSoftObjectPtr<UNiagaraSystem> RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TSoftObjectPtr<UNiagaraSystem> FogSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TSoftObjectPtr<UNiagaraSystem> WindParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TSoftObjectPtr<UNiagaraSystem> VolcanicAshSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TSoftObjectPtr<UNiagaraSystem> PollenSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    TSoftObjectPtr<UNiagaraSystem> InsectSwarmSystem;

    // === AUDIO COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* EnvironmentalAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TSoftObjectPtr<USoundBase> RainAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TSoftObjectPtr<USoundBase> WindAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TSoftObjectPtr<USoundBase> ForestAmbientSound;

    // === ACTIVE COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TArray<UNiagaraComponent*> ActiveParticleSystems;

    // === TIMING AND TRANSITIONS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Timing")
    float WeatherTransitionDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Timing")
    float DayNightCycleDuration = 1200.0f; // 20 minutes real time = 24 hours game time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Timing")
    bool bEnableAutomaticWeatherChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Timing")
    float WeatherChangeInterval = 300.0f; // 5 minutes

private:
    // Internal timing
    float CurrentDayTime = 0.0f;
    float WeatherChangeTimer = 0.0f;
    float TransitionTimer = 0.0f;
    bool bIsTransitioning = false;
    EVFX_CretaceousWeatherType TargetWeather;

public:
    // === PUBLIC INTERFACE ===
    
    UFUNCTION(BlueprintCallable, Category = "Cretaceous Environment")
    void SetWeatherType(EVFX_CretaceousWeatherType NewWeather, bool bInstantTransition = false);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Environment")
    void SetTimeOfDay(EVFX_CretaceousTimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Environment")
    void SpawnEnvironmentalEffect(const FVFX_CretaceousParticleEffect& EffectData);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Environment")
    void CreateCampfireEffect(FVector Location, float IntensityScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Environment")
    void CreateWaterSplashEffect(FVector Location, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Environment")
    void CreateDustCloudEffect(FVector Location, float DustAmount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Environment")
    void CreateBreathVaporEffect(FVector Location, float Temperature = 0.5f);

    UFUNCTION(BlueprintPure, Category = "Cretaceous Environment")
    EVFX_CretaceousWeatherType GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintPure, Category = "Cretaceous Environment")
    EVFX_CretaceousTimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Cretaceous Environment")
    float GetCurrentDayTimeNormalized() const { return CurrentDayTime / DayNightCycleDuration; }

    UFUNCTION(BlueprintPure, Category = "Cretaceous Environment")
    bool IsRaining() const { return CurrentWeather == EVFX_CretaceousWeatherType::LightRain || CurrentWeather == EVFX_CretaceousWeatherType::HeavyRain; }

    UFUNCTION(BlueprintPure, Category = "Cretaceous Environment")
    bool IsFoggy() const { return CurrentWeather == EVFX_CretaceousWeatherType::Fog; }

    UFUNCTION(BlueprintPure, Category = "Cretaceous Environment")
    bool IsWindy() const { return CurrentWeather == EVFX_CretaceousWeatherType::WindStorm; }

protected:
    // === INTERNAL METHODS ===
    
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void UpdateAtmosphericEffects();
    void TransitionToWeather(EVFX_CretaceousWeatherType NewWeather);
    void UpdateAudioForWeather();
    void CleanupExpiredEffects();
    
    // Weather-specific setup methods
    void SetupClearWeather();
    void SetupRainWeather(bool bHeavy = false);
    void SetupFogWeather();
    void SetupWindStormWeather();
    void SetupVolcanicAshWeather();
    
    // Time of day lighting updates
    void UpdateLightingForTimeOfDay();
    FLinearColor GetSunColorForTime() const;
    float GetSunIntensityForTime() const;
    float GetFogDensityForTime() const;
};