#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/DecalActor.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "VFX_ImpactEffectsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<USoundCue> ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSubclassOf<UCameraShakeBase> CameraShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UMaterialInterface> DecalMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DecalSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DecalLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float CameraShakeScale;

    FVFX_ImpactEffectData()
    {
        DecalSize = 100.0f;
        DecalLifetime = 30.0f;
        CameraShakeScale = 1.0f;
    }
};

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    PlayerFootstep      UMETA(DisplayName = "Player Footstep"),
    RockImpact          UMETA(DisplayName = "Rock Impact"),
    TreeFall            UMETA(DisplayName = "Tree Fall"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    DustCloud           UMETA(DisplayName = "Dust Cloud"),
    FireSpark           UMETA(DisplayName = "Fire Spark")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ImpactEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactEffectsManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact Effects")
    TMap<EVFX_ImpactType, FVFX_ImpactEffectData> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxEffectDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxSimultaneousEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableCameraShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableDecals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableAudio;

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveParticleComponents;

    UPROPERTY()
    TArray<ADecalActor*> ActiveDecals;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Impact Effects")
    void PlayImpactEffect(EVFX_ImpactType ImpactType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact Effects")
    void PlayDinosaurFootstep(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact Effects")
    void PlayPlayerFootstep(const FVector& Location, bool bIsRunning = false);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact Effects")
    void PlayBloodSplatter(const FVector& Location, const FVector& Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact Effects")
    void PlayEnvironmentalEffect(EVFX_ImpactType EffectType, const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact Effects")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Impact Effects")
    void SetEffectsEnabled(bool bParticles, bool bDecals, bool bCameraShake, bool bAudio);

private:
    void SpawnParticleEffect(const FVFX_ImpactEffectData& EffectData, const FVector& Location, const FRotator& Rotation, float Scale);
    void SpawnDecalEffect(const FVFX_ImpactEffectData& EffectData, const FVector& Location, const FRotator& Rotation, float Scale);
    void PlayCameraShakeEffect(const FVFX_ImpactEffectData& EffectData, const FVector& Location, float Scale);
    void PlayAudioEffect(const FVFX_ImpactEffectData& EffectData, const FVector& Location, float Scale);
    
    bool IsPlayerInRange(const FVector& Location) const;
    void RemoveOldestEffect();
};