#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "VFX_PrehistoricEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    FootstepLight    UMETA(DisplayName = "Light Footstep"),
    FootstepHeavy    UMETA(DisplayName = "Heavy Footstep"),
    BloodSplatter    UMETA(DisplayName = "Blood Splatter"),
    RockImpact       UMETA(DisplayName = "Rock Impact"),
    WoodImpact       UMETA(DisplayName = "Wood Impact"),
    WaterSplash      UMETA(DisplayName = "Water Splash"),
    DustCloud        UMETA(DisplayName = "Dust Cloud"),
    SparkShower      UMETA(DisplayName = "Spark Shower")
};

UENUM(BlueprintType)
enum class EVFX_EnvironmentType : uint8
{
    Rain             UMETA(DisplayName = "Rain"),
    Snow             UMETA(DisplayName = "Snow"),
    Fog              UMETA(DisplayName = "Fog"),
    WindParticles    UMETA(DisplayName = "Wind Particles"),
    VolcanicAsh      UMETA(DisplayName = "Volcanic Ash"),
    Insects          UMETA(DisplayName = "Insects"),
    Pollen           UMETA(DisplayName = "Pollen"),
    Embers           UMETA(DisplayName = "Fire Embers")
};

USTRUCT(BlueprintType)
struct FVFX_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FLinearColor TintColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoDestroy = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_PrehistoricEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_PrehistoricEffectsManager();

protected:
    virtual void BeginPlay() override;

    // Niagara VFX Systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara VFX")
    TMap<EVFX_ImpactType, TSoftObjectPtr<UNiagaraSystem>> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara VFX")
    TMap<EVFX_EnvironmentType, TSoftObjectPtr<UNiagaraSystem>> EnvironmentEffects;

    // Fallback Particle Systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallback VFX")
    TMap<EVFX_ImpactType, TSoftObjectPtr<UParticleSystem>> FallbackImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fallback VFX")
    TMap<EVFX_EnvironmentType, TSoftObjectPtr<UParticleSystem>> FallbackEnvironmentEffects;

    // Active effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveNiagaraComponents;

    UPROPERTY()
    TArray<UParticleSystemComponent*> ActiveParticleComponents;

public:
    // Impact VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVFX_EffectSettings Settings = FVFX_EffectSettings());

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayFootstepEffect(FVector Location, bool bHeavyStep = false, float DinosaurWeight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayBloodEffect(FVector Location, FVector ImpactDirection, float BloodAmount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayCraftingEffect(FVector Location, EVFX_ImpactType CraftingType);

    // Environment VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void PlayEnvironmentEffect(EVFX_EnvironmentType EnvironmentType, FVector Location, FVFX_EffectSettings Settings = FVFX_EffectSettings());

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StartWeatherEffect(EVFX_EnvironmentType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StopWeatherEffect(EVFX_EnvironmentType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StopAllEffects();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    void SetGlobalVFXQuality(float QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    void CleanupExpiredEffects();

private:
    void InitializeVFXSystems();
    UNiagaraComponent* SpawnNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation, FVFX_EffectSettings Settings);
    UParticleSystemComponent* SpawnParticleEffect(UParticleSystem* System, FVector Location, FRotator Rotation, FVFX_EffectSettings Settings);
    void SetupEffectParameters(UNiagaraComponent* Component, FVFX_EffectSettings Settings);
    void SetupParticleParameters(UParticleSystemComponent* Component, FVFX_EffectSettings Settings);

    // Performance tracking
    UPROPERTY()
    float GlobalVFXQuality = 1.0f;

    UPROPERTY()
    int32 MaxActiveEffects = 50;

    UPROPERTY()
    float EffectCleanupInterval = 5.0f;

    FTimerHandle CleanupTimer;
};