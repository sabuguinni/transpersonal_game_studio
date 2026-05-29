#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "SharedTypes.h"
#include "VFX_EnvironmentalEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EnvironmentalType : uint8
{
    CampfireSmoke,
    MorningMist,
    VolcanicAsh,
    DustDevil,
    RainDroplets,
    WindParticles,
    PollenDrift,
    InsectSwarm
};

USTRUCT(BlueprintType)
struct FVFX_EnvironmentalEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFX_EnvironmentalType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UParticleSystem> LegacyParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsWeatherDependent = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsTimeOfDayDependent = false;

    FVFX_EnvironmentalEffect()
    {
        EffectType = EVFX_EnvironmentalType::CampfireSmoke;
        EffectRadius = 1000.0f;
        EffectIntensity = 1.0f;
        bIsWeatherDependent = true;
        bIsTimeOfDayDependent = false;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_EnvironmentalEffectManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_EnvironmentalEffectManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Environmental effect management
    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnEnvironmentalEffect(EVFX_EnvironmentalType EffectType, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void StopEnvironmentalEffect(EVFX_EnvironmentalType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void UpdateWeatherEffects(EWeatherType WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void UpdateTimeOfDayEffects(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SetEffectQuality(int32 QualityLevel);

protected:
    // Effect configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TArray<FVFX_EnvironmentalEffect> EnvironmentalEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    int32 MaxActiveEffects = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float EffectCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bUseNiagaraWhenAvailable = true;

    // Runtime tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveNiagaraComponents;

    UPROPERTY()
    TArray<UParticleSystemComponent*> ActiveParticleComponents;

    UPROPERTY()
    float LastWeatherUpdate = 0.0f;

    UPROPERTY()
    float LastTimeUpdate = 0.0f;

private:
    void InitializeEffectDatabase();
    void CleanupInactiveEffects();
    UNiagaraComponent* CreateNiagaraEffect(const FVFX_EnvironmentalEffect& Effect, FVector Location);
    UParticleSystemComponent* CreateParticleEffect(const FVFX_EnvironmentalEffect& Effect, FVector Location);
    bool ShouldEffectBeActive(const FVFX_EnvironmentalEffect& Effect) const;
    float GetDistanceToPlayer() const;
};