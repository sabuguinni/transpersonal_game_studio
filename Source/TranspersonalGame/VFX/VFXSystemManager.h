#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire,
    Dust_Footstep,
    Water_Splash,
    Blood_Impact,
    Breath_Vapor,
    Volcanic_Ash,
    Rain_Heavy,
    Wind_Particles
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
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Scale = FVector(1.0f);
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TArray<FVFX_EffectData> EffectDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* Effect);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnDinosaurBreathEffect(FVector Location, FRotator Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnFootstepDustEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void InitializeEffectDatabase();

private:
    FVFX_EffectData* FindEffectData(EVFX_EffectType EffectType);
    void CleanupDestroyedEffects();
};