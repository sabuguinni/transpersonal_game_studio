#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "VFX_NiagaraLibrary.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        NiagaraSystem = nullptr;
        Scale = FVector(1.0f);
        Duration = 3.0f;
        bAutoDestroy = true;
    }
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    Fire_Campfire,
    Dust_FootstepImpact,
    Blood_Splatter,
    Weather_Rain,
    Weather_Snow,
    Volcanic_Ash,
    Water_Splash,
    Breath_Vapor,
    Sparks_Crafting,
    Smoke_Cooking
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Library")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectLibrary;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Library")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* PlayEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void InitializeEffectLibrary();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsEffectActive(EVFX_EffectType EffectType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

private:
    void CleanupFinishedEffects();
    void LoadDefaultEffects();
};