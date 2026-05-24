#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "VFX_CretaceousEnvironmentSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_CretaceousWeatherState : uint8
{
    Clear           UMETA(DisplayName = "Clear Skies"),
    LightFog        UMETA(DisplayName = "Light Morning Fog"),
    HeavyFog        UMETA(DisplayName = "Heavy Fog"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Dust            UMETA(DisplayName = "Dust Storm"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash")
};

UENUM(BlueprintType)
enum class EVFX_TimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Midday          UMETA(DisplayName = "Midday"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FVFX_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogExtinctionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogScatteringDistribution = 0.2f;
};

USTRUCT(BlueprintType)
struct FVFX_ParticleEffectData
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
    bool bAutoActivate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    float LifeSpan = 0.0f; // 0 = infinite
};

/**
 * Manages realistic Cretaceous period environmental VFX systems
 * Handles atmospheric effects, weather particles, and environmental ambience
 * Designed for scientific accuracy and performance optimization
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CretaceousEnvironmentSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CretaceousEnvironmentSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather System
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SetWeatherState(EVFX_CretaceousWeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void TransitionToWeather(EVFX_CretaceousWeatherState TargetWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintPure, Category = "VFX Environment")
    EVFX_CretaceousWeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    // Time of Day System
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SetTimeOfDay(EVFX_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void UpdateAtmosphereForTimeOfDay(EVFX_TimeOfDay TimeOfDay);

    // Atmospheric Effects
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SetAtmosphereSettings(const FVFX_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void CreateVolumetricLightRays(const FVector& LightDirection, float Intensity = 1.0f);

    // Particle Effects Management
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    UNiagaraComponent* SpawnEnvironmentalEffect(const FVFX_ParticleEffectData& EffectData);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void CreateDustCloud(const FVector& Location, float Intensity = 1.0f, float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void CreateWaterSplash(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void CreateCampfireSmoke(const FVector& Location, bool bLargeFireplace = false);

    // Environmental Ambience
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SetForestAmbience(float Density = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void CreateInsectParticles(const FVector& CenterLocation, float Radius = 500.0f);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SetVFXQualityLevel(int32 QualityLevel); // 0=Low, 1=Medium, 2=High, 3=Ultra

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void OptimizeForPerformance(bool bEnableOptimization = true);

protected:
    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    EVFX_CretaceousWeatherState CurrentWeatherState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    EVFX_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    FVFX_AtmosphereSettings CurrentAtmosphereSettings;

    // Weather Transition
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    bool bIsTransitioning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    float TransitionProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    float TransitionDuration;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    EVFX_CretaceousWeatherState TargetWeatherState;

    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TObjectPtr<UExponentialHeightFogComponent> FogComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TArray<TObjectPtr<UNiagaraComponent>> ActiveParticleEffects;

    // Asset References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> DustCloudSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> WaterSplashSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> CampfireSmokeSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> FogSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> VolcanicAshSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> InsectSwarmSystem;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveParticleEffects = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float EffectCullDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 CurrentQualityLevel = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    bool bPerformanceOptimizationEnabled = true;

private:
    // Internal Methods
    void UpdateWeatherTransition(float DeltaTime);
    void CleanupExpiredEffects();
    void UpdateEffectLOD();
    UExponentialHeightFogComponent* FindOrCreateFogComponent();
    void ApplyQualitySettings();
    void LoadRequiredAssets();
};