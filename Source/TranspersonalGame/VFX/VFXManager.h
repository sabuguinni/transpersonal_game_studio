#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Niagara/Public/NiagaraSystem.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "VFXManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<USoundBase> Sound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float VolumeMultiplier;

    FVFX_EffectData()
    {
        Duration = 3.0f;
        VolumeMultiplier = 1.0f;
    }
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire,
    Fire_Torch,
    Water_Rain,
    Water_Splash,
    Dust_Footstep,
    Dust_Impact,
    Blood_Splatter,
    Blood_Drip,
    Smoke_Fire,
    Smoke_Cooking,
    Sparks_Fire,
    Sparks_Tool,
    Wind_Leaves,
    Wind_Dust
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_Manager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UVFX_Manager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, const FVector& Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(EVFX_EffectType EffectType, USceneComponent* AttachToComponent, FName AttachPointName = NAME_None, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, const FVector& Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXOfType(EVFX_EffectType EffectType);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartRainEffect(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopRainEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateFootstepDust(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateBloodSplatter(const FVector& Location, const FVector& Direction, float Amount = 1.0f);

    // Fire VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* CreateCampfire(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ExtinguishFire(UNiagaraComponent* FireComponent);

protected:
    // VFX Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<EVFX_EffectType, FVFX_EffectData> VFXDatabase;

    // Active VFX tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    UPROPERTY()
    TMap<EVFX_EffectType, TArray<UNiagaraComponent*>> ActiveVFXByType;

    // Environmental state
    UPROPERTY()
    UNiagaraComponent* RainComponent;

    UPROPERTY()
    bool bIsRaining;

private:
    void InitializeVFXDatabase();
    void CleanupInactiveVFX();
    UAudioComponent* PlayVFXSound(const FVFX_EffectData& EffectData, const FVector& Location);
};