#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFX_CombatParticleSystem.generated.h"

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CombatParticleSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CombatParticleSystem();

protected:
    virtual void BeginPlay() override;

    // Niagara systems for combat effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat VFX")
    class UNiagaraSystem* BloodImpactEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat VFX")
    class UNiagaraSystem* DustCloudEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat VFX")
    class UNiagaraSystem* SparkEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat VFX")
    class UNiagaraSystem* WaterSplashEffect;

public:
    // Combat effect triggers
    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void TriggerBloodImpact(FVector Location, FVector Normal, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void TriggerDustCloud(FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void TriggerSparks(FVector Location, FVector Direction, int32 SparkCount = 10);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void TriggerWaterSplash(FVector Location, float Velocity = 500.0f);

    // Dinosaur-specific effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerDinosaurBreath(FVector Location, FVector Direction, bool bColdWeather = false);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerDinosaurRoar(FVector Location, float IntensityRadius = 1000.0f);

private:
    // Internal effect management
    void SpawnNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector::OneVector);
    
    // Effect pooling for performance
    UPROPERTY()
    TArray<class UNiagaraComponent*> EffectPool;
    
    int32 PoolIndex;
    static const int32 MAX_POOL_SIZE = 50;
};