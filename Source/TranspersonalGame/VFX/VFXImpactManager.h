#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "VFXImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    FootstepDust,
    BloodSplatter,
    RockImpact,
    WaterSplash,
    ClawStrike
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> VFXSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<USoundBase> ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float VFXScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float SoundVolume = 1.0f;

    FVFX_ImpactData()
    {
        VFXScale = 1.0f;
        SoundVolume = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact", meta = (AllowPrivateAccess = "true"))
    TMap<EVFX_ImpactType, FVFX_ImpactData> ImpactVFXMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* ActiveVFXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* ImpactAudioComponent;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayImpactVFX(EVFX_ImpactType ImpactType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, float ScaleMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetupImpactData(EVFX_ImpactType ImpactType, UNiagaraSystem* VFXSystem, USoundBase* Sound, float Scale = 1.0f, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool HasImpactData(EVFX_ImpactType ImpactType) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "VFX")
    void InitializeDefaultVFX();

private:
    void LoadDefaultVFXSystems();
    void SpawnVFXAtLocation(UNiagaraSystem* VFXSystem, const FVector& Location, const FRotator& Rotation, float Scale);
    void PlaySoundAtLocation(USoundBase* Sound, const FVector& Location, float Volume);
};