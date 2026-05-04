#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    DinosaurLanding     UMETA(DisplayName = "Dinosaur Landing"),
    WeaponHit           UMETA(DisplayName = "Weapon Hit"),
    RockImpact          UMETA(DisplayName = "Rock Impact"),
    TreeFall            UMETA(DisplayName = "Tree Fall"),
    WaterSplash         UMETA(DisplayName = "Water Splash")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float EffectScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float VolumeMultiplier;

    FVFX_ImpactData()
    {
        ParticleEffect = nullptr;
        ImpactSound = nullptr;
        EffectScale = 1.0f;
        VolumeMultiplier = 1.0f;
    }
};

/**
 * Manages impact VFX and audio for dinosaur footsteps, weapon hits, and environmental interactions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ImpactManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Impact effect spawning
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnImpactEffect(EVFX_ImpactType ImpactType, const FVector& Location, const FVector& Normal = FVector::UpVector, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnDinosaurFootstep(const FVector& Location, float DinosaurSize = 1.0f, ESurfaceType SurfaceType = ESurfaceType::Grass);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnWeaponImpact(const FVector& Location, const FVector& Normal, float Damage = 1.0f);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SetImpactData(EVFX_ImpactType ImpactType, const FVFX_ImpactData& NewData);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    FVFX_ImpactData GetImpactData(EVFX_ImpactType ImpactType) const;

protected:
    // Impact effect data for each type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact Data")
    TMap<EVFX_ImpactType, FVFX_ImpactData> ImpactEffects;

    // Global settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalVFXScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableAudio;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinTimeBetweenEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects;

private:
    // Active effect tracking
    TArray<class UNiagaraComponent*> ActiveParticleComponents;
    TArray<class UAudioComponent*> ActiveAudioComponents;
    
    float LastEffectTime;

    // Helper functions
    void CleanupFinishedEffects();
    bool CanSpawnNewEffect() const;
    void InitializeDefaultEffects();
};