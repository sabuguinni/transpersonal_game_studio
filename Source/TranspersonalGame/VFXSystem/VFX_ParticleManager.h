#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "VFX_ParticleManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire,
    Blood_Impact,
    Dust_Footstep,
    Water_Splash,
    Smoke_Ambient,
    Sparks_Crafting,
    Rain_Weather,
    Mist_Environment
};

USTRUCT(BlueprintType)
struct FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Lifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float SpawnRate = 100.0f;

    FVFX_EffectConfig()
    {
        Scale = FVector(1.0f);
        Lifetime = 5.0f;
        bAutoDestroy = true;
        SpawnRate = 100.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ParticleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXAtLocation(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXAttached(EVFX_EffectType EffectType, USceneComponent* AttachComponent, FName SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    // Preset effect functions for common scenarios
    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    void CreateCampfire(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    void CreateBloodSplatter(const FVector& ImpactLocation, const FVector& ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    void CreateFootstepDust(const FVector& FootLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    void CreateCraftingSparks(const FVector& CraftLocation);

    // Configuration and management
    UFUNCTION(BlueprintCallable, Category = "VFX Config")
    void SetEffectConfig(EVFX_EffectType EffectType, const FVFX_EffectConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "VFX Config")
    FVFX_EffectConfig GetEffectConfig(EVFX_EffectType EffectType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQualityLevel(int32 QualityLevel);

protected:
    // Effect configuration storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    TMap<EVFX_EffectType, FVFX_EffectConfig> EffectConfigs;

    // Active effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Quality settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentQualityLevel = 2; // 0=Low, 1=Medium, 2=High

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullDistance = 5000.0f;

private:
    void InitializeDefaultConfigs();
    void CleanupFinishedEffects();
    bool ShouldCullEffect(UNiagaraComponent* Effect) const;
    UNiagaraComponent* CreateNiagaraComponent(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation);
};