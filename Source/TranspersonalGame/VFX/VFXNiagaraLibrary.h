#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXNiagaraLibrary.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Fire_Torch          UMETA(DisplayName = "Torch"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Snow        UMETA(DisplayName = "Snow"),
    Weather_Fog         UMETA(DisplayName = "Fog"),
    Dino_Footstep       UMETA(DisplayName = "Dinosaur Footstep"),
    Dino_Breath         UMETA(DisplayName = "Dinosaur Breath"),
    Blood_Impact        UMETA(DisplayName = "Blood Impact"),
    Dust_Cloud          UMETA(DisplayName = "Dust Cloud"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Sparks_Crafting     UMETA(DisplayName = "Crafting Sparks"),
    Smoke_General       UMETA(DisplayName = "General Smoke")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        DefaultScale = FVector(1.0f);
        DefaultLifetime = 5.0f;
        bAutoDestroy = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachToComponent, FName AttachPointName = NAME_None, FVector LocationOffset = FVector::ZeroVector, FRotator RotationOffset = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterEffectData(EVFX_EffectType EffectType, const FVFX_EffectData& EffectData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsEffectRegistered(EVFX_EffectType EffectType) const;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectDatabase;

    void InitializeDefaultEffects();
    UNiagaraComponent* CreateNiagaraComponent(const FVFX_EffectData& EffectData, FVector Location, FRotator Rotation, FVector Scale);

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};