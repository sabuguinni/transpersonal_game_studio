#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "VFX_ImpactEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    None            UMETA(DisplayName = "None"),
    DinosaurFootstep UMETA(DisplayName = "Dinosaur Footstep"),
    PlayerFootstep  UMETA(DisplayName = "Player Footstep"),
    WeaponImpact    UMETA(DisplayName = "Weapon Impact"),
    BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
    RockCrush       UMETA(DisplayName = "Rock Crush"),
    WaterSplash     UMETA(DisplayName = "Water Splash")
};

UENUM(BlueprintType)
enum class EVFX_SurfaceType : uint8
{
    Dirt        UMETA(DisplayName = "Dirt"),
    Rock        UMETA(DisplayName = "Rock"),
    Grass       UMETA(DisplayName = "Grass"),
    Water       UMETA(DisplayName = "Water"),
    Mud         UMETA(DisplayName = "Mud"),
    Sand        UMETA(DisplayName = "Sand"),
    Wood        UMETA(DisplayName = "Wood")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UParticleSystem> LegacyParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAttachToSurface = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectDuration = 3.0f;

    FVFX_ImpactEffect()
    {
        EffectScale = 1.0f;
        VolumeMultiplier = 1.0f;
        bAttachToSurface = false;
        EffectDuration = 3.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ImpactEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core impact effect spawning
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnImpactEffect(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, 
                          const FVector& Location, const FVector& Normal, 
                          float IntensityMultiplier = 1.0f);

    // Dinosaur-specific footstep effects
    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void SpawnDinosaurFootstep(const FVector& Location, const FVector& Normal, 
                              float DinosaurSize = 1.0f, const FString& DinosaurType = "Generic");

    // Player interaction effects
    UFUNCTION(BlueprintCallable, Category = "VFX Player")
    void SpawnPlayerFootstep(const FVector& Location, EVFX_SurfaceType SurfaceType);

    // Combat and weapon effects
    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void SpawnWeaponImpact(const FVector& Location, const FVector& Normal, 
                          const FString& WeaponType = "Spear");

    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void SpawnBloodEffect(const FVector& Location, const FVector& Normal, 
                         float BloodAmount = 1.0f);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnDustCloud(const FVector& Location, float Radius = 200.0f, 
                       float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnGroundCrack(const FVector& StartLocation, const FVector& EndLocation, 
                         float CrackWidth = 50.0f);

    // Configuration and management
    UFUNCTION(BlueprintCallable, Category = "VFX Config")
    void SetGlobalVFXIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Config")
    void EnableVFXLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "VFX Config")
    void SetMaxActiveEffects(int32 MaxEffects);

protected:
    // Effect configuration maps
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    TMap<EVFX_ImpactType, FVFX_ImpactEffect> ImpactEffectMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    TMap<EVFX_SurfaceType, FVFX_ImpactEffect> SurfaceEffectMap;

    // Global VFX settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalVFXIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableVFXLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectCullDistance = 5000.0f;

    // Active effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveNiagaraEffects;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioEffects;

    // Performance monitoring
    UPROPERTY()
    int32 CurrentActiveEffects = 0;

    UPROPERTY()
    float LastCleanupTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float CleanupInterval = 5.0f;

private:
    // Internal helper methods
    void InitializeEffectMaps();
    void CleanupFinishedEffects();
    bool ShouldSpawnEffect(const FVector& Location) const;
    float CalculateLODMultiplier(const FVector& Location) const;
    void RegisterActiveEffect(UNiagaraComponent* Effect);
    void RegisterActiveAudio(UAudioComponent* Audio);
};