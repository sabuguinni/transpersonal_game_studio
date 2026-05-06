#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "VFX_ImpactManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<USoundCue> ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector EffectScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAttachToSurface;

    FVFX_ImpactData()
    {
        EffectScale = FVector(1.0f, 1.0f, 1.0f);
        EffectDuration = 3.0f;
        bAttachToSurface = false;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ImpactManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Impact effect mapping by surface type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    TMap<TEnumAsByte<EPhysicalSurface>, FVFX_ImpactData> SurfaceImpacts;

    // Dinosaur-specific impact effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Dinosaur")
    FVFX_ImpactData FootstepEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Dinosaur")
    FVFX_ImpactData TailSwipeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Dinosaur")
    FVFX_ImpactData BiteEffect;

    // Combat impact effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Combat")
    FVFX_ImpactData BloodSplatterEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Combat")
    FVFX_ImpactData WeaponImpactEffect;

    // Environmental effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    FVFX_ImpactData RockFallEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    FVFX_ImpactData TreeFallEffect;

    // Public interface functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayImpactEffect(const FVector& Location, const FVector& Normal, EPhysicalSurface SurfaceType, float ImpactForce = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayFootstepEffect(const FVector& Location, const FVector& Normal, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayBloodEffect(const FVector& Location, const FVector& Direction, float BloodAmount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayWeaponImpact(const FVector& Location, const FVector& Normal, EPhysicalSurface SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayEnvironmentalEffect(const FVector& Location, bool bIsRockFall = false);

protected:
    // Internal helper functions
    void SpawnParticleEffect(const FVFX_ImpactData& EffectData, const FVector& Location, const FRotator& Rotation, float ScaleMultiplier = 1.0f);
    
    void PlayImpactSound(const FVFX_ImpactData& EffectData, const FVector& Location, float VolumeMultiplier = 1.0f);

    FRotator CalculateEffectRotation(const FVector& Normal);

private:
    // Active effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance optimization
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxConcurrentEffects;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float EffectCullDistance;

    void CleanupFinishedEffects();
    bool ShouldCullEffect(const FVector& EffectLocation) const;
};