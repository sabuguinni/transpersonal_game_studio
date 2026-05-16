#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "VFX_ParticleSystemManager.generated.h"

// VFX particle type enum for prehistoric effects
UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    VolcanicEruption    UMETA(DisplayName = "Volcanic Eruption"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    DustCloud           UMETA(DisplayName = "Dust Cloud"),
    DinosaurBreath      UMETA(DisplayName = "Dinosaur Breath"),
    CampfireSmoke       UMETA(DisplayName = "Campfire Smoke"),
    BloodSpray          UMETA(DisplayName = "Blood Spray"),
    FootstepDust        UMETA(DisplayName = "Footstep Dust"),
    WeatherRain         UMETA(DisplayName = "Weather Rain"),
    WeatherSnow         UMETA(DisplayName = "Weather Snow"),
    InsectSwarm         UMETA(DisplayName = "Insect Swarm")
};

// VFX intensity levels for realistic prehistoric effects
UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Low         UMETA(DisplayName = "Low Intensity"),
    Medium      UMETA(DisplayName = "Medium Intensity"),
    High        UMETA(DisplayName = "High Intensity"),
    Extreme     UMETA(DisplayName = "Extreme Intensity")
};

// Particle system configuration for prehistoric VFX
USTRUCT(BlueprintType)
struct FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TSoftObjectPtr<UNiagaraSystem> ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_IntensityLevel Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy;

    FVFX_ParticleConfig()
    {
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Intensity = EVFX_IntensityLevel::Medium;
        Duration = 5.0f;
        Scale = 1.0f;
        bLooping = false;
        bAutoDestroy = true;
    }
};

/**
 * VFX Particle System Manager - Handles all prehistoric visual effects
 * Creates realistic particle effects for dinosaur interactions, environmental events,
 * and atmospheric conditions in the prehistoric world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ParticleSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Niagara component for particle effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* MainParticleComponent;

    // Audio component for VFX sound synchronization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UAudioComponent* VFXAudioComponent;

    // Particle system configurations for each effect type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<EVFX_ParticleType, FVFX_ParticleConfig> ParticleConfigs;

    // Active particle components for management
    UPROPERTY(BlueprintReadOnly, Category = "VFX Runtime")
    TArray<class UNiagaraComponent*> ActiveParticleComponents;

    // Current biome for biome-specific VFX adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    EBiomeType CurrentBiome;

    // Weather intensity for environmental VFX
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherIntensity;

    // Time of day for day/night VFX adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay;

public:
    // Spawn particle effect at specified location
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    class UNiagaraComponent* SpawnParticleEffect(EVFX_ParticleType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    // Stop specific particle effect
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopParticleEffect(class UNiagaraComponent* ParticleComponent);

    // Stop all active particle effects
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopAllParticleEffects();

    // Create volcanic eruption effect with audio
    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void TriggerVolcanicEruption(FVector EruptionLocation, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::High);

    // Create water splash effect for dinosaur water entry
    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void TriggerWaterSplash(FVector SplashLocation, float DinosaurSize = 1.0f);

    // Create dust cloud effect for dinosaur movement
    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void TriggerDustCloud(FVector FootstepLocation, float DinosaurWeight = 1.0f);

    // Create dinosaur breath vapor effect
    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void TriggerDinosaurBreath(FVector MouthLocation, FRotator BreathDirection);

    // Create blood spray effect for combat
    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void TriggerBloodSpray(FVector ImpactLocation, FVector ImpactDirection, EVFX_IntensityLevel Severity = EVFX_IntensityLevel::Medium);

    // Create campfire smoke and ember effects
    UFUNCTION(BlueprintCallable, Category = "VFX Survival")
    void TriggerCampfireEffect(FVector FireLocation, bool bIsLit = true);

    // Update environmental VFX based on biome and weather
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void UpdateEnvironmentalVFX(EBiomeType NewBiome, float NewWeatherIntensity, float NewTimeOfDay);

    // Create weather particle effects (rain, snow)
    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void TriggerWeatherEffect(EVFX_ParticleType WeatherType, EVFX_IntensityLevel Intensity);

    // Create insect swarm effect for forest biome
    UFUNCTION(BlueprintCallable, Category = "VFX Biome")
    void TriggerInsectSwarm(FVector SwarmCenter, float SwarmRadius = 500.0f);

protected:
    // Initialize particle system configurations
    UFUNCTION(CallInEditor)
    void InitializeParticleConfigs();

    // Clean up expired particle effects
    void CleanupExpiredEffects();

    // Get biome-specific particle intensity multiplier
    float GetBiomeIntensityMultiplier(EVFX_ParticleType EffectType) const;

    // Get time-of-day particle visibility multiplier
    float GetTimeOfDayMultiplier(EVFX_ParticleType EffectType) const;

    // Load Niagara system asset safely
    UNiagaraSystem* LoadParticleSystemAsset(const FString& AssetPath) const;
};