#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Engine/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "VFX_NiagaraEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    Footstep        UMETA(DisplayName = "Footstep Impact"),
    Campfire        UMETA(DisplayName = "Campfire"),
    Blood           UMETA(DisplayName = "Blood Splatter"),
    Dust            UMETA(DisplayName = "Dust Cloud"),
    Rain            UMETA(DisplayName = "Rain"),
    Fog             UMETA(DisplayName = "Fog"),
    Sparks          UMETA(DisplayName = "Sparks"),
    Smoke           UMETA(DisplayName = "Smoke"),
    Breath          UMETA(DisplayName = "Breath Vapor"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    RockDebris      UMETA(DisplayName = "Rock Debris"),
    LeafParticles   UMETA(DisplayName = "Leaf Particles"),
    Embers          UMETA(DisplayName = "Fire Embers")
};

UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Duration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector LocationOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FRotator RotationOffset = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAttachToActor = false;

    FVFX_EffectSettings()
    {
        EffectType = EVFX_EffectType::None;
        Intensity = EVFX_IntensityLevel::Medium;
        Duration = 3.0f;
        Scale = 1.0f;
        LocationOffset = FVector::ZeroVector;
        RotationOffset = FRotator::ZeroRotator;
        bAutoDestroy = true;
        bAttachToActor = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ActiveEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Effect")
    UNiagaraComponent* NiagaraComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Effect")
    UParticleSystemComponent* ParticleComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Effect")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Effect")
    float RemainingDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Effect")
    bool bIsActive = false;

    FVFX_ActiveEffect()
    {
        NiagaraComponent = nullptr;
        ParticleComponent = nullptr;
        EffectType = EVFX_EffectType::None;
        RemainingDuration = 0.0f;
        bIsActive = false;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_NiagaraEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SpawnEffect(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, const FVFX_EffectSettings& Settings = FVFX_EffectSettings());

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SpawnEffectAtActor(EVFX_EffectType EffectType, AActor* TargetActor, const FVFX_EffectSettings& Settings = FVFX_EffectSettings());

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void StopEffect(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void StopAllEffects();

    // Dinosaur-Specific Effects
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void CreateFootstepImpact(const FVector& Location, float DinosaurMass = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void CreateBreathVapor(AActor* DinosaurActor, float Temperature = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void CreateBloodSplatter(const FVector& Location, const FVector& ImpactDirection);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void CreateCampfire(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void CreateRainEffect(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void CreateFogEffect(float Density = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void CreateWaterSplash(const FVector& Location, float SplashSize = 1.0f);

    // Combat Effects
    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void CreateWeaponImpact(const FVector& Location, const FVector& ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void CreateCraftingSparks(const FVector& Location);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODLevel(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateEffectLOD(float DistanceToPlayer);

protected:
    // Effect Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> FootstepDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> CampfireSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> BloodSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> SmokeSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> SparksSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> WaterSplashSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> BreathVaporSystem;

    // Active Effects Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    TArray<FVFX_ActiveEffect> ActiveEffects;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentLODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDistanceCulling = true;

    // Environmental State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float CurrentWindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float AmbientTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float AmbientHumidity = 0.6f;

private:
    // Internal Methods
    UNiagaraComponent* CreateNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation, const FVFX_EffectSettings& Settings);
    void UpdateActiveEffects(float DeltaTime);
    void CleanupFinishedEffects();
    UNiagaraSystem* GetEffectSystem(EVFX_EffectType EffectType);
    void ApplyLODSettings(UNiagaraComponent* Component, int32 LODLevel);
    bool ShouldCullEffect(const FVector& EffectLocation);

    // Timer Handles
    FTimerHandle EffectCleanupTimer;
    FTimerHandle LODUpdateTimer;
};