#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "VFX_EffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    DustImpact,
    BloodSplatter,
    CampfireFire,
    FootstepDust,
    WeatherRain,
    WeatherSnow,
    VolcanicAsh,
    WaterSplash,
    CombatSparks,
    BreathVapor
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        Scale = FVector(1.0f);
        Duration = 3.0f;
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_EffectManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UVFX_EffectManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXAtLocation(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* AttachVFXToActor(EVFX_EffectType EffectType, AActor* TargetActor, const FName& SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(class UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterEffectData(EVFX_EffectType EffectType, const FVFX_EffectData& EffectData);

    // Gameplay-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Gameplay")
    void PlayFootstepVFX(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Gameplay")
    void PlayCombatImpactVFX(const FVector& Location, const FVector& ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX|Gameplay")
    void PlayCampfireVFX(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StartWeatherVFX(EVFX_EffectType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StopWeatherVFX();

protected:
    // Effect registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectRegistry;

    // Active effects tracking
    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveEffects;

    // Weather VFX
    UPROPERTY()
    class UNiagaraComponent* CurrentWeatherVFX;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float EffectCullDistance = 10000.0f;

private:
    void InitializeDefaultEffects();
    void CleanupExpiredEffects();
    bool CanSpawnNewEffect() const;
    void CullDistantEffects();

    FTimerHandle CleanupTimerHandle;
};