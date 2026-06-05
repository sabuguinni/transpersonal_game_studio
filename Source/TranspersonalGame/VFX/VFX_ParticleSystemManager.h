#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "VFX_ParticleSystemManager.generated.h"

/**
 * VFX Particle System Manager - Handles all Niagara particle effects for prehistoric survival game
 * Creates realistic environmental effects: footstep dust, combat impacts, weather particles
 * Optimized with LOD system and effect pooling for performance
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

    // === CORE COMPONENTS ===
    
    /** Root component for positioning */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Core")
    USceneComponent* RootSceneComponent;
    
    /** Visual marker for editor identification */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Core")
    UStaticMeshComponent* VisualMarker;

    // === PARTICLE EFFECT POOLS ===
    
    /** Pool of Niagara components for footstep dust effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Pools")
    TArray<UNiagaraComponent*> FootstepDustPool;
    
    /** Pool of Niagara components for combat impact effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Pools")
    TArray<UNiagaraComponent*> CombatImpactPool;
    
    /** Pool of Niagara components for environmental effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Pools")
    TArray<UNiagaraComponent*> EnvironmentalPool;

    // === NIAGARA SYSTEM REFERENCES ===
    
    /** Niagara system for dinosaur footstep dust clouds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* FootstepDustSystem;
    
    /** Niagara system for blood impact effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* BloodImpactSystem;
    
    /** Niagara system for weapon impact sparks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* WeaponImpactSystem;
    
    /** Niagara system for environmental dust and debris */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* EnvironmentalDustSystem;
    
    /** Niagara system for rain and weather effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* WeatherRainSystem;

    // === EFFECT SETTINGS ===
    
    /** Maximum number of simultaneous particle effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings", meta = (ClampMin = "5", ClampMax = "50"))
    int32 MaxSimultaneousEffects;
    
    /** Distance beyond which effects are culled for performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings", meta = (ClampMin = "500", ClampMax = "5000"))
    float EffectCullingDistance;
    
    /** Scale multiplier for all particle effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float GlobalEffectScale;

public:
    // === PUBLIC EFFECT TRIGGERING FUNCTIONS ===
    
    /**
     * Trigger footstep dust effect at specified location
     * @param Location World location to spawn dust effect
     * @param Intensity Effect intensity (0.0-1.0)
     * @param DinosaurSize Size multiplier based on dinosaur type
     */
    UFUNCTION(BlueprintCallable, Category = "VFX Triggers")
    void TriggerFootstepDust(const FVector& Location, float Intensity = 1.0f, float DinosaurSize = 1.0f);
    
    /**
     * Trigger combat impact effect (blood, sparks, debris)
     * @param Location Impact location
     * @param ImpactType Type of impact (weapon, bite, claw)
     * @param Intensity Damage-based intensity
     * @param SurfaceType Surface material hit
     */
    UFUNCTION(BlueprintCallable, Category = "VFX Triggers")
    void TriggerCombatImpact(const FVector& Location, EVFXImpactType ImpactType, float Intensity = 1.0f, EVFXSurfaceType SurfaceType = EVFXSurfaceType::Flesh);
    
    /**
     * Trigger environmental effect (wind dust, falling debris)
     * @param Location Effect location
     * @param EffectType Type of environmental effect
     * @param Duration Effect duration in seconds
     */
    UFUNCTION(BlueprintCallable, Category = "VFX Triggers")
    void TriggerEnvironmentalEffect(const FVector& Location, EVFXEnvironmentalType EffectType, float Duration = 3.0f);
    
    /**
     * Start weather particle system (rain, snow, dust storm)
     * @param WeatherType Type of weather effect
     * @param Intensity Weather intensity (0.0-1.0)
     */
    UFUNCTION(BlueprintCallable, Category = "VFX Triggers")
    void StartWeatherEffect(EVFXWeatherType WeatherType, float Intensity = 1.0f);
    
    /**
     * Stop all weather effects
     */
    UFUNCTION(BlueprintCallable, Category = "VFX Triggers")
    void StopWeatherEffects();

protected:
    // === INTERNAL EFFECT MANAGEMENT ===
    
    /** Initialize particle effect pools */
    void InitializeEffectPools();
    
    /** Get available Niagara component from pool */
    UNiagaraComponent* GetAvailableComponent(TArray<UNiagaraComponent*>& Pool);
    
    /** Return component to pool after effect completes */
    void ReturnComponentToPool(UNiagaraComponent* Component, TArray<UNiagaraComponent*>& Pool);
    
    /** Cull distant effects for performance */
    void CullDistantEffects();
    
    /** Timer handle for effect cleanup */
    FTimerHandle EffectCleanupTimer;
    
    /** Cleanup completed effects */
    UFUNCTION()
    void CleanupCompletedEffects();

private:
    /** Track active effects for performance monitoring */
    int32 ActiveEffectCount;
    
    /** Player pawn reference for distance culling */
    APawn* PlayerPawn;
    
    /** Find and cache player pawn reference */
    void CachePlayerPawn();
};