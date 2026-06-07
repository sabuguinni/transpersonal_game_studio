#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFX_NiagaraLibrary.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Fire_Torch          UMETA(DisplayName = "Torch"),
    Dust_Footstep       UMETA(DisplayName = "Footstep Dust"),
    Dust_DinoStomp      UMETA(DisplayName = "Dinosaur Stomp"),
    Blood_Impact        UMETA(DisplayName = "Blood Impact"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Smoke_Cooking       UMETA(DisplayName = "Cooking Smoke"),
    Rain_Heavy          UMETA(DisplayName = "Heavy Rain"),
    Fog_Morning         UMETA(DisplayName = "Morning Fog")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy;

    FVFX_EffectData()
    {
        Scale = FVector(1.0f);
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(VFX))
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
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterEffect(EVFX_EffectType EffectType, UNiagaraSystem* NiagaraSystem, FVector Scale = FVector(1.0f), float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool HasEffect(EVFX_EffectType EffectType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

private:
    void CleanupFinishedEffects();
    void InitializeDefaultEffects();
};