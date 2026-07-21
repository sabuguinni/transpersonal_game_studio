#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "VFX_CretaceousAtmosphereManager.generated.h"

/**
 * VFX Manager for Cretaceous period atmospheric effects
 * Handles dust particles, pollen, atmospheric haze, and volumetric lighting
 * Integrates with Audio Agent's day/night cycle system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_CretaceousAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_CretaceousAtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Core")
    class USceneComponent* RootSceneComponent;

    // Atmospheric particle systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric VFX")
    class UNiagaraComponent* AtmosphericDustComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric VFX")
    class UNiagaraComponent* PollenParticlesComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric VFX")
    class UNiagaraComponent* VolumetricHazeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric VFX")
    class UNiagaraComponent* InsectSwarmComponent;

    // Niagara system assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* AtmosphericDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* PollenSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* VolumetricHazeSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* InsectSwarmSystem;

    // Atmospheric parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float AtmosphericDensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float WindStrength = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float HumidityLevel = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings", meta = (ClampMin = "15.0", ClampMax = "45.0"))
    float TemperatureCelsius = 28.0f;

    // Time of day integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Integration")
    bool bSyncWithDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Integration", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxParticleCount = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float ParticleViewDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem = true;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void SetAtmosphericDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void SetWindStrength(float NewWindStrength);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void SetHumidityLevel(float NewHumidity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void UpdateTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void SetWeatherCondition(const FString& WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(int32 PerformanceLevel);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void StartAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric VFX")
    void StopAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SyncWithAudioDayNightCycle(float AudioTimeOfDay, float AudioWindStrength);

private:
    // Internal functions
    void InitializeNiagaraSystems();
    void UpdateParticleParameters();
    void UpdateWindEffects();
    void UpdateLightingIntegration();
    void OptimizePerformance();

    // Performance tracking
    float LastPerformanceCheck = 0.0f;
    int32 CurrentParticleCount = 0;
    bool bPerformanceOptimizationActive = false;

    // Integration with other systems
    class ADirectionalLight* MainDirectionalLight;
    class AAudio_DayNightCycleManager* DayNightCycleManager;
};