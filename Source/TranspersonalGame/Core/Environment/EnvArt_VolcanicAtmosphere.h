#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "EnvArt_VolcanicAtmosphere.generated.h"

/**
 * Volcanic Atmosphere Controller
 * Manages atmospheric effects specific to volcanic biomes including:
 * - Ash particle systems
 * - Heat haze distortion effects
 * - Lava glow lighting
 * - Atmospheric color grading for volcanic environments
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_VolcanicAtmosphere : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_VolcanicAtmosphere();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Volcanic Atmosphere")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Volcanic Atmosphere")
    class UPostProcessComponent* VolcanicPostProcess;

    // Ash Particle System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ash Effects")
    class UParticleSystemComponent* AshParticleSystem;

    // Lava Glow Lighting
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lava Lighting")
    class UPointLightComponent* LavaGlowLight;

    // Heat Haze Effect Zone
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heat Effects")
    class UStaticMeshComponent* HeatHazeZone;

    // Volcanic Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float VolcanicIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float AshDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float LavaGlowIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    FLinearColor LavaGlowColor = FLinearColor(1.0f, 0.3f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float HeatHazeStrength = 0.8f;

    // Atmospheric Color Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Colors")
    FLinearColor VolcanicTint = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Colors")
    float AtmosphericOpacity = 0.3f;

    // Dynamic Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    bool bEnableDynamicAsh = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    bool bEnableHeatDistortion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    float EffectRadius = 2000.0f;

    // Timing and Animation
    float TimeAccumulator = 0.0f;
    float AshFlickerTimer = 0.0f;
    float HeatWaveTimer = 0.0f;

public:
    // Volcanic Atmosphere Control Functions
    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void SetVolcanicIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void SetAshDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void SetLavaGlowProperties(float Intensity, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void SetHeatHazeStrength(float Strength);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void EnableVolcanicEffects(bool bEnable);

    // Atmospheric State Functions
    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void UpdateAtmosphericTint(FLinearColor NewTint, float Opacity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void TriggerAshBurst(float BurstIntensity = 2.0f, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void ModulateHeatEffects(float HeatLevel);

    // Environmental Integration
    UFUNCTION(BlueprintCallable, Category = "Environmental Integration")
    void IntegrateWithBiomeSystem(class AWorld_BiomeSystem* BiomeSystem);

    UFUNCTION(BlueprintCallable, Category = "Environmental Integration")
    void SynchronizeWithWeatherSystem(class AEnvArt_WeatherSystem* WeatherSystem);

    // Performance and LOD
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetEffectLOD(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForDistance(float DistanceToPlayer);

protected:
    // Internal Update Functions
    void UpdateAshParticles(float DeltaTime);
    void UpdateLavaGlow(float DeltaTime);
    void UpdateHeatHaze(float DeltaTime);
    void UpdateAtmosphericPostProcess(float DeltaTime);

    // Effect Management
    void InitializeVolcanicEffects();
    void ConfigurePostProcessSettings();
    void SetupAshParticleSystem();
    void ConfigureLavaLighting();
    void SetupHeatHazeZone();

    // Performance Optimization
    void UpdateEffectLOD();
    bool ShouldUpdateEffects() const;
    float GetDistanceToPlayer() const;

private:
    // Internal state tracking
    bool bEffectsInitialized = false;
    int32 CurrentLODLevel = 0;
    float LastPlayerDistance = 0.0f;
    
    // Effect timing
    float AshBurstTimer = 0.0f;
    float AshBurstDuration = 0.0f;
    float AshBurstIntensity = 1.0f;
    bool bInAshBurst = false;
};