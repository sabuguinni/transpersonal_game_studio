#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    DustCloud       UMETA(DisplayName = "Dust Cloud"),
    BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
    CampfireFire    UMETA(DisplayName = "Campfire Fire"),
    FootstepImpact  UMETA(DisplayName = "Footstep Impact"),
    WeatherRain     UMETA(DisplayName = "Weather Rain"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* NiagaraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UParticleSystem* LegacyEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::DustCloud;
        NiagaraEffect = nullptr;
        LegacyEffect = nullptr;
        Duration = 2.0f;
        Intensity = 1.0f;
        bLooping = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_Manager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_Manager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    TArray<FVFX_EffectData> AvailableEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TArray<UNiagaraComponent*> NiagaraComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TArray<UParticleSystemComponent*> ParticleComponents;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayDinosaurFootstepEffect(FVector ImpactLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayBloodSplatterEffect(FVector HitLocation, FVector HitNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayCampfireEffect(FVector FireLocation, bool bStartFire = true);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectIntensity(EVFX_EffectType EffectType, float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsEffectPlaying(EVFX_EffectType EffectType) const;

protected:
    void InitializeEffectComponents();
    UNiagaraComponent* GetNiagaraComponentForEffect(EVFX_EffectType EffectType);
    UParticleSystemComponent* GetParticleComponentForEffect(EVFX_EffectType EffectType);
    
    FVFX_EffectData* GetEffectData(EVFX_EffectType EffectType);
};

#include "VFXManager.generated.h"