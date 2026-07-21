#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFX_ImpactEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    WeaponHit          UMETA(DisplayName = "Weapon Hit"),
    RockImpact         UMETA(DisplayName = "Rock Impact"),
    TreeFall           UMETA(DisplayName = "Tree Fall"),
    BodySlam           UMETA(DisplayName = "Body Slam"),
    ClawStrike         UMETA(DisplayName = "Claw Strike")
};

UENUM(BlueprintType)
enum class EVFX_SurfaceType : uint8
{
    Dirt               UMETA(DisplayName = "Dirt"),
    Rock               UMETA(DisplayName = "Rock"),
    Grass              UMETA(DisplayName = "Grass"),
    Sand               UMETA(DisplayName = "Sand"),
    Mud                UMETA(DisplayName = "Mud"),
    Water              UMETA(DisplayName = "Water"),
    Wood               UMETA(DisplayName = "Wood")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Effect")
    TSoftObjectPtr<UNiagaraSystem> ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Effect")
    TSoftObjectPtr<USoundCue> ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Effect")
    float ParticleScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Effect")
    float SoundVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Effect")
    float EffectDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Effect")
    bool bCreateDecal = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Effect")
    FLinearColor DecalColor = FLinearColor::Brown;

    FVFX_ImpactEffect()
    {
        ParticleScale = 1.0f;
        SoundVolume = 1.0f;
        EffectDuration = 2.0f;
        bCreateDecal = true;
        DecalColor = FLinearColor::Brown;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ImpactEffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactEffectsManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Impact effect mappings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Effects")
    TMap<EVFX_ImpactType, FVFX_ImpactEffect> ImpactEffectMap;

    // Surface-specific effect variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Effects")
    TMap<EVFX_SurfaceType, FVFX_ImpactEffect> SurfaceEffectMap;

    // Active effect components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effects")
    TArray<UNiagaraComponent*> ActiveParticleEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effects")
    TArray<UAudioComponent*> ActiveAudioEffects;

    // Effect settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxSimultaneousEffects = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float EffectCullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableImpactDecals = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DecalLifetime = 30.0f;

public:
    // Main impact effect function
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerImpactEffect(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, 
                           FVector Location, FVector Normal, float Intensity = 1.0f);

    // Dinosaur-specific impact effects
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f, 
                               EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDinosaurBodySlam(FVector Location, FVector Normal, float DinosaurSize = 1.0f);

    // Weapon impact effects
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerWeaponImpact(FVector Location, FVector Normal, float WeaponDamage = 1.0f,
                           EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt);

    // Environmental impact effects
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerRockImpact(FVector Location, FVector Normal, float RockSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerTreeFall(FVector Location, FVector Direction, float TreeSize = 1.0f);

    // Effect management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    int32 GetActiveEffectCount() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "VFX Configuration")
    void SetEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Configuration")
    void SetMaxSimultaneousEffects(int32 MaxEffects);

    UFUNCTION(BlueprintCallable, Category = "VFX Configuration")
    void EnableImpactDecals(bool bEnable);

private:
    // Internal effect creation
    void CreateParticleEffect(const FVFX_ImpactEffect& EffectData, FVector Location, 
                            FVector Normal, float Scale);
    
    void CreateAudioEffect(const FVFX_ImpactEffect& EffectData, FVector Location, float Volume);
    
    void CreateDecalEffect(const FVFX_ImpactEffect& EffectData, FVector Location, 
                         FVector Normal, float Scale);

    // Effect cleanup
    void RemoveExpiredEffects();
    bool IsEffectExpired(UNiagaraComponent* Effect, float CurrentTime) const;
    bool IsAudioExpired(UAudioComponent* Audio) const;

    // Distance culling
    bool ShouldCullEffect(FVector EffectLocation) const;
    FVector GetPlayerLocation() const;

    // Effect initialization
    void InitializeDefaultEffects();
    void LoadEffectAssets();

    // Effect intensity scaling
    float GlobalEffectIntensity = 1.0f;
    float CalculateScaledIntensity(float BaseIntensity) const;
};