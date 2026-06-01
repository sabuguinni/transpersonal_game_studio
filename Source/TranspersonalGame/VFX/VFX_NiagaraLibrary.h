#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "VFX_NiagaraLibrary.generated.h"

// VFX Categories for prehistoric survival gameplay
UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    Combat          UMETA(DisplayName = "Combat"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Weather         UMETA(DisplayName = "Weather"),
    Fire            UMETA(DisplayName = "Fire"),
    Impact          UMETA(DisplayName = "Impact")
};

// VFX Intensity levels for performance scaling
UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Low             UMETA(DisplayName = "Low"),
    Medium          UMETA(DisplayName = "Medium"),
    High            UMETA(DisplayName = "High"),
    Epic            UMETA(DisplayName = "Epic")
};

// VFX Effect data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_IntensityLevel IntensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping;

    FVFX_EffectData()
    {
        EffectName = TEXT("DefaultEffect");
        Category = EVFX_EffectCategory::Environment;
        Duration = 2.0f;
        IntensityLevel = EVFX_IntensityLevel::Medium;
        bLooping = false;
    }
};

/**
 * Niagara VFX Library for prehistoric survival game effects
 * Manages all particle systems for realistic environmental and combat VFX
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(const FVector& Location, EVFX_EffectCategory Category, const FString& EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(USceneComponent* AttachComponent, EVFX_EffectCategory Category, const FString& EffectName);

    // Dinosaur-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayFootstepEffect(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayRoarEffect(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayBreathingEffect(USceneComponent* AttachPoint, bool bColdWeather = false);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void PlayBloodImpact(const FVector& Location, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void PlayWeaponImpact(const FVector& Location, const FString& WeaponType);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void PlayCampfireEffect(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void PlayDustCloud(const FVector& Location, float Size = 1.0f);

    // Weather VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StartRainEffect(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StopRainEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void PlayLightningEffect(const FVector& Location);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetGlobalVFXQuality(EVFX_IntensityLevel QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void CleanupExpiredEffects();

protected:
    // VFX effect library
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Library")
    TMap<FString, FVFX_EffectData> EffectLibrary;

    // Active effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EVFX_IntensityLevel CurrentQualityLevel;

private:
    // Internal helper functions
    void InitializeEffectLibrary();
    UNiagaraComponent* CreateNiagaraComponent(const FVFX_EffectData& EffectData);
    void RegisterActiveEffect(UNiagaraComponent* Effect);
    void RemoveInactiveEffects();
    bool ShouldCullEffect(const FVector& EffectLocation) const;
};