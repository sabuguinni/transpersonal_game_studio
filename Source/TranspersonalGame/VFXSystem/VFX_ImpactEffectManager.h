#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "TranspersonalGame/SharedTypes.h"
#include "VFX_ImpactEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    FootstepLight,
    FootstepHeavy,
    FootstepMassive,
    WeaponHit,
    RockImpact,
    WoodImpact,
    FleshImpact,
    GroundCrack
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<USoundCue> ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float SoundVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectDuration = 3.0f;

    FVFX_ImpactData()
    {
        EffectScale = 1.0f;
        SoundVolume = 1.0f;
        EffectDuration = 3.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ImpactEffectManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactEffectManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact Effects")
    TMap<EVFX_ImpactType, FVFX_ImpactData> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxEffectDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects = 20;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveSounds;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FVector Normal = FVector::UpVector, float ScaleMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayFootstepEffect(FVector Location, float DinosaurMass = 1000.0f, ESurfaceType SurfaceType = ESurfaceType::Dirt);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayWeaponImpact(FVector Location, FVector Normal, EWeaponType WeaponType, ESurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CleanupExpiredEffects();

private:
    void InitializeDefaultEffects();
    EVFX_ImpactType GetFootstepTypeFromMass(float Mass);
    void SpawnParticleEffect(UNiagaraSystem* Effect, FVector Location, FRotator Rotation, float Scale);
    void PlayImpactSound(USoundCue* Sound, FVector Location, float Volume);
    void RemoveOldestEffect();
};