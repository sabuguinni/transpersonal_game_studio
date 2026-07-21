#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "SharedTypes.h"
#include "VFX_ParticleEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    FootstepDust        UMETA(DisplayName = "Footstep Dust"),
    CampfireFire        UMETA(DisplayName = "Campfire Fire"),
    CampfireSmoke       UMETA(DisplayName = "Campfire Smoke"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    RockDebris          UMETA(DisplayName = "Rock Debris"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    DinosaurBreath      UMETA(DisplayName = "Dinosaur Breath"),
    WeatherRain         UMETA(DisplayName = "Weather Rain"),
    WeatherFog          UMETA(DisplayName = "Weather Fog"),
    VolcanicAsh         UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::FootstepDust;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Duration = 3.0f;
        Intensity = 1.0f;
        bAutoDestroy = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ParticleEffectManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleEffectManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepDust(FVector FootLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfire(FVector FireLocation, bool bIncludeSmoke = true);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodEffect(FVector ImpactLocation, FVector ImpactDirection, float BloodAmount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnRockDebris(FVector ImpactLocation, float DebrisAmount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWaterSplash(FVector WaterLocation, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurBreath(FVector MouthLocation, FRotator BreathDirection, float BreathIntensity = 1.0f);

    // Weather and environmental effects
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartRainEffect(float RainIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopRainEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartFogEffect(float FogDensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopFogEffect();

    // Effect management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

protected:
    // Effect asset references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* FootstepDustEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* CampfireFireEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* CampfireSmokeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* BloodSplatterEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* RockDebrisEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* WaterSplashEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* DinosaurBreathEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* RainEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* FogEffect;

    // Active effect tracking
    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    TArray<float> EffectTimers;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCleanupInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DefaultEffectDuration = 5.0f;

private:
    float CleanupTimer = 0.0f;

    // Helper functions
    class UNiagaraComponent* CreateNiagaraEffect(class UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation, float Duration);
    void LoadVFXAssets();
    void RegisterActiveEffect(class UNiagaraComponent* Effect, float Duration);
    void RemoveExpiredEffect(int32 Index);
};