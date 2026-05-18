#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/ParticleSystemComponent.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    BloodSplatter,
    FootstepDust,
    VolcanicAsh,
    CampfireSmoke,
    RainDrops,
    DinosaurBreath
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UParticleSystem> ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector DefaultScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::BloodSplatter;
        DefaultScale = FVector(1.0f);
        Duration = 2.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_Manager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_Manager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    TArray<FVFX_EffectData> EffectDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveNiagaraEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UParticleSystemComponent*> ActiveParticleEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(FVector ImpactLocation, FVector ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepDust(FVector FootLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVolcanicAsh(FVector VolcanoLocation, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

private:
    FVFX_EffectData* GetEffectData(EVFX_EffectType EffectType);
    
    void InitializeEffectDatabase();
};