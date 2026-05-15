#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "VFX_ParticleEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None UMETA(DisplayName = "None"),
    FootstepImpact UMETA(DisplayName = "Footstep Impact"),
    CampfireFlames UMETA(DisplayName = "Campfire Flames"),
    BloodSplatter UMETA(DisplayName = "Blood Splatter"),
    DustCloud UMETA(DisplayName = "Dust Cloud"),
    WeaponImpact UMETA(DisplayName = "Weapon Impact"),
    BreathVapor UMETA(DisplayName = "Breath Vapor"),
    WaterSplash UMETA(DisplayName = "Water Splash"),
    RockDebris UMETA(DisplayName = "Rock Debris")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<USoundBase> SoundEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        Duration = 2.0f;
        Scale = 1.0f;
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ParticleEffectManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleEffectManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime") 
    TArray<UAudioComponent*> ActiveAudioEffects;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float CustomScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffectAtLocation(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachComponent, FName AttachPointName = NAME_None);

    // Dinosaur-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void SpawnFootstepEffect(FVector FootLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur") 
    void SpawnBreathVaporEffect(FVector MouthLocation, FRotator BreathDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void SpawnBloodEffect(FVector ImpactLocation, FVector ImpactNormal);

    // Environment VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnCampfireEffect(FVector FireLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnDustCloudEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnWaterSplashEffect(FVector WaterLocation, float SplashSize = 1.0f);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void SpawnWeaponImpactEffect(FVector ImpactLocation, FVector ImpactNormal, EVFX_EffectType WeaponType);

    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void SpawnRockDebrisEffect(FVector Location, FVector Direction);

    // Management functions
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management") 
    void CleanupFinishedEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    int32 GetActiveEffectCount() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "VFX Config")
    void SetEffectData(EVFX_EffectType EffectType, const FVFX_EffectData& EffectData);

    UFUNCTION(BlueprintCallable, Category = "VFX Config")
    FVFX_EffectData GetEffectData(EVFX_EffectType EffectType) const;

private:
    void InitializeDefaultEffects();
    void PlayEffectAudio(const FVFX_EffectData& EffectData, FVector Location);
    void CleanupEffect(UNiagaraComponent* Effect);
    void CleanupAudioEffect(UAudioComponent* AudioEffect);

    UPROPERTY()
    float EffectCleanupTimer = 0.0f;

    static constexpr float CLEANUP_INTERVAL = 1.0f;
    static constexpr int32 MAX_ACTIVE_EFFECTS = 50;
};