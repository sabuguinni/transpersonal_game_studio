#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "VFX_NiagaraSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire Fire"),
    Dust_Footstep       UMETA(DisplayName = "Dinosaur Footstep Dust"),
    Blood_Impact        UMETA(DisplayName = "Blood Splatter"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Weather_Rain        UMETA(DisplayName = "Rain Particles"),
    Smoke_Fire          UMETA(DisplayName = "Fire Smoke"),
    Sparks_Impact       UMETA(DisplayName = "Stone Sparks"),
    Breath_Cold         UMETA(DisplayName = "Cold Breath Vapor")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = -1.0f; // -1 = infinite

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoActivate = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Scale = FVector::OneVector;
        Duration = -1.0f;
        bAutoActivate = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_NiagaraSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TArray<FVFX_EffectData> EffectDataArray;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveNiagaraComponents;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void InitializeVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* NiagaraComp);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    TArray<UNiagaraComponent*> GetActiveVFXComponents() const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXScale(UNiagaraComponent* NiagaraComp, FVector NewScale);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXLocation(UNiagaraComponent* NiagaraComp, FVector NewLocation);

private:
    FVFX_EffectData* GetEffectDataByType(EVFX_EffectType EffectType);
    void LoadDefaultNiagaraSystems();
    UNiagaraSystem* CreatePlaceholderNiagaraSystem(EVFX_EffectType EffectType);
};