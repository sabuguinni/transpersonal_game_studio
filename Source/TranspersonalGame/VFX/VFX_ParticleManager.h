#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "../SharedTypes.h"
#include "VFX_ParticleManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    None = 0,
    FootstepDust,
    CampfireFire,
    CampfireSmoke,
    BloodSplatter,
    DinosaurBreath,
    WeatherRain,
    WeatherSnow,
    VolcanicAsh,
    WaterSplash,
    RockImpact
};

USTRUCT(BlueprintType)
struct FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_ParticleType ParticleType = EVFX_ParticleType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FLinearColor TintColor = FLinearColor::White;

    FVFX_ParticleConfig()
    {
        ParticleType = EVFX_ParticleType::None;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 2.0f;
        bAutoDestroy = true;
        TintColor = FLinearColor::White;
    }
};

/**
 * VFX Particle Manager - Handles all particle effects for the prehistoric survival game
 * Creates realistic environmental effects: dust, fire, smoke, blood, weather
 * NO mystical/spiritual effects - only physically plausible prehistoric world VFX
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ParticleManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UVFX_ParticleManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Particle spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UParticleSystemComponent* SpawnParticleEffect(EVFX_ParticleType ParticleType, 
        FVector Location, FRotator Rotation = FRotator::ZeroRotator, 
        const FVFX_ParticleConfig& Config = FVFX_ParticleConfig());

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepDust(FVector Location, EBiomeType BiomeType, float IntensityScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(FVector Location, FVector Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurBreath(FVector Location, FRotator Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherEffect(EVFX_ParticleType WeatherType, FVector Location, float Radius = 1000.0f);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

protected:
    // Particle system references
    UPROPERTY(EditDefaultsOnly, Category = "VFX Assets")
    TMap<EVFX_ParticleType, TSoftObjectPtr<class UParticleSystem>> ParticleSystemMap;

    UPROPERTY(EditDefaultsOnly, Category = "VFX Assets")
    TMap<EVFX_ParticleType, TSoftObjectPtr<class UNiagaraSystem>> NiagaraSystemMap;

    // Active effect tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<UParticleSystemComponent>> ActiveEffects;

    UPROPERTY()
    TArray<TWeakObjectPtr<UNiagaraComponent>> ActiveNiagaraEffects;

private:
    void LoadDefaultParticleSystems();
    UParticleSystem* GetParticleSystemForType(EVFX_ParticleType ParticleType);
    FLinearColor GetBiomeParticleColor(EBiomeType BiomeType);
    float GetBiomeParticleScale(EBiomeType BiomeType);
};