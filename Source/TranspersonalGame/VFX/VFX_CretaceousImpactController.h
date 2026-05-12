#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "VFXTypes.h"
#include "VFX_CretaceousImpactController.generated.h"

/**
 * VFX_CretaceousImpactController
 * Manages realistic impact effects for the Cretaceous period survival game.
 * Handles footsteps, weapon impacts, creature collisions, and environmental interactions.
 * All effects are scientifically plausible for the prehistoric setting.
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CretaceousImpactController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CretaceousImpactController();

protected:
    virtual void BeginPlay() override;

public:
    // === IMPACT EFFECT TRIGGERS ===
    
    /** Trigger footstep impact effect based on surface type and creature size */
    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void TriggerFootstepImpact(FVector Location, float CreatureWeight, EVFX_SurfaceType SurfaceType, float Velocity = 1.0f);
    
    /** Trigger weapon impact effect (spear, stone, club) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void TriggerWeaponImpact(FVector Location, FVector Normal, EVFX_WeaponType WeaponType, EVFX_SurfaceType SurfaceType);
    
    /** Trigger creature collision impact (dinosaur vs dinosaur, dinosaur vs environment) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void TriggerCreatureImpact(FVector Location, float ImpactForce, EVFX_CreatureSize CreatureSize);
    
    /** Trigger environmental destruction (tree falling, rock breaking) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void TriggerEnvironmentalDestruction(FVector Location, EVFX_DestructionType DestructionType, float Scale = 1.0f);

protected:
    // === NIAGARA PARTICLE SYSTEMS ===
    
    /** Footstep dust particles for different surface types */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Particles")
    TMap<EVFX_SurfaceType, TSoftObjectPtr<UNiagaraSystem>> FootstepParticleSystems;
    
    /** Weapon impact sparks and debris */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Particles")
    TMap<EVFX_WeaponType, TSoftObjectPtr<UNiagaraSystem>> WeaponImpactSystems;
    
    /** Creature collision effects */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Particles")
    TSoftObjectPtr<UNiagaraSystem> CreatureCollisionSystem;
    
    /** Environmental destruction particles */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Particles")
    TMap<EVFX_DestructionType, TSoftObjectPtr<UNiagaraSystem>> DestructionSystems;

    // === SOUND EFFECTS ===
    
    /** Footstep audio for different surfaces */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Audio")
    TMap<EVFX_SurfaceType, TSoftObjectPtr<USoundCue>> FootstepSounds;
    
    /** Weapon impact audio */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Audio")
    TMap<EVFX_WeaponType, TSoftObjectPtr<USoundCue>> WeaponImpactSounds;

    // === PERFORMANCE SETTINGS ===
    
    /** Maximum number of simultaneous impact effects */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance", meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxSimultaneousEffects = 20;
    
    /** Distance beyond which impact effects are culled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance", meta = (ClampMin = "500", ClampMax = "10000"))
    float CullingDistance = 3000.0f;
    
    /** Scale factor for particle density based on performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float ParticleDensityScale = 1.0f;

private:
    // === INTERNAL MANAGEMENT ===
    
    /** Active Niagara components for cleanup */
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveComponents;
    
    /** Timer for cleaning up finished effects */
    FTimerHandle CleanupTimerHandle;
    
    /** Clean up finished particle systems */
    void CleanupFinishedEffects();
    
    /** Get appropriate particle system for surface type and impact type */
    UNiagaraSystem* GetParticleSystemForImpact(EVFX_SurfaceType SurfaceType, EVFX_ImpactType ImpactType);
    
    /** Calculate particle scale based on creature weight and impact force */
    float CalculateParticleScale(float Weight, float Force);
    
    /** Check if location is within culling distance */
    bool IsWithinCullingDistance(FVector Location);
};