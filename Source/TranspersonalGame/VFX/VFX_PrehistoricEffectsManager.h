#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "VFX_PrehistoricEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_PrehistoricEffectType : uint8
{
    FootstepDust        UMETA(DisplayName = "Footstep Dust"),
    CampfireFire        UMETA(DisplayName = "Campfire Fire"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    WeaponImpact        UMETA(DisplayName = "Weapon Impact"),
    BreathVapor         UMETA(DisplayName = "Breath Vapor"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    VolcanicAsh         UMETA(DisplayName = "Volcanic Ash"),
    WindParticles       UMETA(DisplayName = "Wind Particles")
};

USTRUCT(BlueprintType)
struct FVFX_PrehistoricEffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_PrehistoricEffectType EffectType = EVFX_PrehistoricEffectType::FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAttachToActor = false;

    FVFX_PrehistoricEffectSettings()
    {
        EffectType = EVFX_PrehistoricEffectType::FootstepDust;
        Duration = 2.0f;
        Intensity = 1.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        bAutoDestroy = true;
        bAttachToActor = false;
    }
};

UCLASS(BlueprintType, Blueprintable, Category = "VFX")
class TRANSPERSONALGAME_API UVFX_PrehistoricEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_PrehistoricEffectsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main VFX spawning function
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnPrehistoricEffect(
        EVFX_PrehistoricEffectType EffectType,
        FVector Location,
        FRotator Rotation = FRotator::ZeroRotator,
        const FVFX_PrehistoricEffectSettings& Settings = FVFX_PrehistoricEffectSettings()
    );

    // Specific effect spawners
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepDust(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfire(FVector Location, float FireIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(FVector Location, FVector ImpactDirection, float Amount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeaponImpact(FVector Location, FVector Normal, bool bIsStone = true);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBreathVapor(FVector Location, float Temperature = 0.0f);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWaterSplash(FVector Location, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVolcanicAsh(FVector Location, FVector WindDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWindParticles(FVector Location, FVector WindDirection, float Intensity = 1.0f);

    // Cleanup and management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

protected:
    // Niagara system references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* FootstepDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* CampfireSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* BloodSplatterSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* WeaponImpactSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* BreathVaporSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* WaterSplashSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* VolcanicAshSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* WindParticlesSystem;

    // Active effect tracking
    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveEffects;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxEffectDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableVFXLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float LODDistanceNear = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float LODDistanceFar = 3000.0f;

private:
    // Internal helper functions
    UNiagaraSystem* GetSystemForEffectType(EVFX_PrehistoricEffectType EffectType);
    void ApplyLODSettings(class UNiagaraComponent* Component, float Distance);
    void RegisterActiveEffect(class UNiagaraComponent* Component);
    void RemoveExpiredEffect(class UNiagaraComponent* Component);

    // Timer handles
    FTimerHandle CleanupTimerHandle;
};

UCLASS(BlueprintType, Blueprintable, Category = "VFX")
class TRANSPERSONALGAME_API AVFX_PrehistoricEffectsActor : public AActor
{
    GENERATED_BODY()

public:
    AVFX_PrehistoricEffectsActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // VFX Manager Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UVFX_PrehistoricEffectsManager* VFXManager;

    // Test functions for editor
    UFUNCTION(CallInEditor)
    void TestFootstepEffect();

    UFUNCTION(CallInEditor)
    void TestCampfireEffect();

    UFUNCTION(CallInEditor)
    void TestAllEffects();

protected:
    // Root component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;
};