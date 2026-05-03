#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "SharedTypes.h"
#include "VFXManager.generated.h"

// VFX Categories for prehistoric survival game
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    DinosaurFootstep,
    DinosaurBreath,
    DinosaurImpact,
    CampfireFire,
    CampfireSmoke,
    WeatherRain,
    WeatherSnow,
    WeatherFog,
    CombatBlood,
    CombatImpact,
    EnvironmentDust,
    EnvironmentWater,
    CraftingSparks
};

// VFX Effect Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

/**
 * VFX Manager - Handles all visual effects for prehistoric survival game
 * Manages Niagara particle systems for dinosaur impacts, weather, fire, combat
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_Manager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_Manager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurBreathEffect(FVector Location, FRotator Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCombatImpactEffect(FVector Location, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Intensity = 1.0f);

    // VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffectsByType(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount() const;

protected:
    // VFX System Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<EVFX_EffectType, TSoftObjectPtr<UNiagaraSystem>> VFXSystems;

    // Active VFX Components
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    // VFX Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveVFXEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float VFXCullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableVFXLOD = true;

private:
    // Internal functions
    void InitializeVFXSystems();
    void CleanupFinishedVFX();
    UNiagaraSystem* GetVFXSystemForType(EVFX_EffectType EffectType);
    bool ShouldCullVFXAtLocation(FVector Location);
};