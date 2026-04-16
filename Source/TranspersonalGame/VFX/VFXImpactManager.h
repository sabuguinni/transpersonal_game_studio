#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "VFXImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    PlayerFootstep      UMETA(DisplayName = "Player Footstep"),
    WeaponImpact        UMETA(DisplayName = "Weapon Impact"),
    RockFall            UMETA(DisplayName = "Rock Fall"),
    TreeFall            UMETA(DisplayName = "Tree Fall"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    BloodSpatter        UMETA(DisplayName = "Blood Spatter"),
    DustCloud           UMETA(DisplayName = "Dust Cloud")
};

UENUM(BlueprintType)
enum class EVFX_SurfaceType : uint8
{
    Dirt                UMETA(DisplayName = "Dirt"),
    Rock                UMETA(DisplayName = "Rock"),
    Mud                 UMETA(DisplayName = "Mud"),
    Sand                UMETA(DisplayName = "Sand"),
    Grass               UMETA(DisplayName = "Grass"),
    Water               UMETA(DisplayName = "Water"),
    Wood                UMETA(DisplayName = "Wood"),
    Bone                UMETA(DisplayName = "Bone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    EVFX_ImpactType ImpactType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    EVFX_SurfaceType SurfaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ParticleScale;

    FVFX_ImpactData()
    {
        ImpactType = EVFX_ImpactType::PlayerFootstep;
        SurfaceType = EVFX_SurfaceType::Dirt;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactForce = 1.0f;
        ParticleScale = 1.0f;
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

    // Root component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Niagara systems for different impact types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<EVFX_ImpactType, UNiagaraSystem*> ImpactSystems;

    // Surface-specific particle effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<EVFX_SurfaceType, UNiagaraSystem*> SurfaceEffects;

    // Sound effects for impacts
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EVFX_ImpactType, USoundCue*> ImpactSounds;

    // Active Niagara components pool
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UNiagaraComponent*> ActiveEffects;

    // Pool of reusable Niagara components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UNiagaraComponent*> EffectPool;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 PoolSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectLifetime;

public:
    // Main impact creation function
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateImpactEffect(const FVFX_ImpactData& ImpactData);

    // Specific impact functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateFootstepEffect(FVector Location, EVFX_SurfaceType Surface, float CreatureSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateWeaponImpact(FVector Location, FVector Normal, EVFX_SurfaceType Surface, float Force = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateBloodEffect(FVector Location, FVector Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateDustCloud(FVector Location, float Radius = 100.0f, float Duration = 3.0f);

    // Pool management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void InitializeEffectPool();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* GetPooledEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ReturnEffectToPool(UNiagaraComponent* Effect);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetMaxActiveEffects(int32 NewMax);

protected:
    // Internal helper functions
    void PlayImpactSound(const FVFX_ImpactData& ImpactData);
    UNiagaraSystem* GetSystemForImpact(const FVFX_ImpactData& ImpactData);
    void ConfigureEffectParameters(UNiagaraComponent* Effect, const FVFX_ImpactData& ImpactData);

    // Timer for cleanup
    FTimerHandle CleanupTimer;

    // Effect cleanup callback
    UFUNCTION()
    void OnEffectFinished(UNiagaraComponent* FinishedEffect);
};