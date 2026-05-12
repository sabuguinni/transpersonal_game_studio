#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    Light_Footstep      UMETA(DisplayName = "Light Footstep"),
    Medium_Footstep     UMETA(DisplayName = "Medium Footstep"), 
    Heavy_Footstep      UMETA(DisplayName = "Heavy Footstep"),
    Massive_Footstep    UMETA(DisplayName = "Massive Footstep"),
    Weapon_Impact       UMETA(DisplayName = "Weapon Impact"),
    Fall_Impact         UMETA(DisplayName = "Fall Impact"),
    Rock_Impact         UMETA(DisplayName = "Rock Impact")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* DustEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* DebrisEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    USoundCue* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float ParticleScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float SpawnRadius;

    FVFX_ImpactData()
    {
        DustEffect = nullptr;
        DebrisEffect = nullptr;
        ImpactSound = nullptr;
        ParticleScale = 1.0f;
        EffectDuration = 2.0f;
        SpawnRadius = 100.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TMap<EVFX_ImpactType, FVFX_ImpactData> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float MinTimeBetweenEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    bool bUseGroundMaterialDetection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float EffectCullDistance;

private:
    float LastEffectTime;
    UWorld* CachedWorld;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FVector Normal = FVector::UpVector, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerFootstepEffect(float DinosaurMass, FVector Location, FVector Normal = FVector::UpVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetupDefaultEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    EVFX_ImpactType GetImpactTypeFromMass(float Mass);

private:
    bool ShouldSpawnEffect(FVector Location);
    void SpawnDustEffect(UNiagaraSystem* Effect, FVector Location, FVector Normal, float Scale);
    void SpawnDebrisEffect(UNiagaraSystem* Effect, FVector Location, FVector Normal, float Scale);
    void PlayImpactSound(USoundCue* Sound, FVector Location, float VolumeMultiplier);
};