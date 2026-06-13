#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "SharedTypes.h"
#include "VFX_ParticleManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ParticleManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Core")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Core")
    class UNiagaraComponent* PrimaryParticleSystem;

    // VFX Categories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Fire")
    class UNiagaraSystem* CampfireEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Fire")
    class UNiagaraSystem* EmberEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    class UNiagaraSystem* DustImpactEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    class UNiagaraSystem* FootstepDustEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Weather")
    class UNiagaraSystem* RainEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Weather")
    class UNiagaraSystem* FogEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Combat")
    class UNiagaraSystem* BloodSplatterEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Combat")
    class UNiagaraSystem* ImpactSparkEffect;

    // VFX Control Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float EffectIntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableWeatherEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableCombatEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableEnvironmentalEffects;

    // VFX Methods
    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    void SpawnCampfireEffect(FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnDustImpact(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnFootstepDust(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void StartRainEffect(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void StopRainEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void SpawnBloodSplatter(FVector Location, FVector Direction, float Amount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void SpawnImpactSparks(FVector Location, FVector Normal);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SetGlobalVFXIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void EnableVFXCategory(EVFX_EffectCategory Category, bool bEnable);

private:
    // Internal VFX tracking
    TArray<class UNiagaraComponent*> ActiveParticleSystems;
    
    float CurrentWeatherIntensity;
    bool bIsRaining;
    
    // Performance optimization
    int32 MaxActiveParticles;
    float EffectCullDistance;
    
    void CleanupInactiveEffects();
    void UpdatePerformanceSettings();
    bool ShouldSpawnEffect(FVector Location) const;
};