#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "VFX_EffectManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* NiagaraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class USoundCue* SoundEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping;

    FVFX_EffectData()
    {
        NiagaraEffect = nullptr;
        SoundEffect = nullptr;
        Duration = 1.0f;
        Intensity = 1.0f;
        bLooping = false;
    }
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    BloodImpact = 1,
    DustCloud = 2,
    Sparks = 3,
    Fire = 4,
    Water = 5,
    Smoke = 6,
    DinosaurBreath = 7,
    FootstepDust = 8,
    WeaponImpact = 9,
    EnvironmentalMist = 10
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_EffectManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EffectManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Effect Data Storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectLibrary;

    // Active Effects Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<class UNiagaraComponent*> ActiveEffects;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float EffectCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float UpdateFrequency;

public:
    // Main VFX Interface
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayEffectAtActor(EVFX_EffectType EffectType, AActor* TargetActor, FVector Offset = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    // Dinosaur-Specific Effects
    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void PlayDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void PlayDinosaurRoarEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void PlayDinosaurBreathEffect(AActor* DinosaurActor);

    // Combat Effects
    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void PlayBloodImpact(FVector ImpactLocation, FVector ImpactNormal, float Damage = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void PlayWeaponImpact(FVector ImpactLocation, FVector ImpactNormal, EWeaponType WeaponType);

    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void PlayDamageFlash(AActor* DamagedActor, float DamageAmount);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void PlayCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void PlayWaterSplash(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void PlayEnvironmentalMist(FVector Location, float Radius = 500.0f);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void CullDistantEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void SetEffectQuality(float QualityLevel);

private:
    // Internal Methods
    UNiagaraComponent* CreateEffectComponent(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float Scale);
    void CleanupFinishedEffects();
    bool IsLocationVisible(FVector Location) const;
    void InitializeEffectLibrary();

    // Timer Handles
    FTimerHandle EffectCleanupTimer;
    FTimerHandle PerformanceCullTimer;

    // Runtime State
    float LastUpdateTime;
    class APlayerController* CachedPlayerController;
};