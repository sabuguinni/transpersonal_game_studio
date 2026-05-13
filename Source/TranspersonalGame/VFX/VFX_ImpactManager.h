#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    FootstepDust,
    BloodSplatter,
    RockDebris,
    WaterSplash,
    GroundCrack,
    VegetationRustle
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAttachToGround = true;

    FVFX_ImpactData()
    {
        Scale = FVector(1.0f);
        Duration = 3.0f;
        bAttachToGround = true;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact Systems")
    TMap<EVFX_ImpactType, FVFX_ImpactData> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveEffects = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float EffectCullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float MinTimeBetweenEffects = 0.1f;

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    float LastEffectTime = 0.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnImpactEffect(EVFX_ImpactType ImpactType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, float ScaleMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnFootstepEffect(const FVector& FootLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnBloodEffect(const FVector& ImpactLocation, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void SetMaxActiveEffects(int32 NewMax);

    UFUNCTION(BlueprintPure, Category = "VFX Performance")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

protected:
    void InitializeDefaultEffects();
    bool CanSpawnEffect() const;
    void RemoveExpiredEffect(UNiagaraComponent* Effect);
};