#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SharedTypes.h"
#include "VFXManager.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class AActor;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_FootstepData
{
    GENERATED_BODY()

    // Sistema de partículas para pegadas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    TSoftObjectPtr<UNiagaraSystem> FootstepParticleSystem;

    // Intensidade do efeito baseada no peso do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float ImpactIntensity = 1.0f;

    // Duração do efeito em segundos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float EffectDuration = 2.0f;

    // Raio de partículas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float ParticleRadius = 100.0f;

    FVFX_FootstepData()
    {
        ImpactIntensity = 1.0f;
        EffectDuration = 2.0f;
        ParticleRadius = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_WeatherData
{
    GENERATED_BODY()

    // Sistema de partículas para chuva
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    TSoftObjectPtr<UNiagaraSystem> RainSystem;

    // Sistema de partículas para neve
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    TSoftObjectPtr<UNiagaraSystem> SnowSystem;

    // Sistema de partículas para nevoeiro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    TSoftObjectPtr<UNiagaraSystem> FogSystem;

    // Intensidade do tempo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float WeatherIntensity = 0.5f;

    FVFX_WeatherData()
    {
        WeatherIntensity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_CombatData
{
    GENERATED_BODY()

    // Sistema de partículas para sangue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    TSoftObjectPtr<UNiagaraSystem> BloodSplatterSystem;

    // Sistema de partículas para impacto de armas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    TSoftObjectPtr<UNiagaraSystem> WeaponImpactSystem;

    // Sistema de partículas para poeira de combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    TSoftObjectPtr<UNiagaraSystem> CombatDustSystem;

    FVFX_CombatData()
    {
    }
};

/**
 * VFX Manager - Sistema central de gestão de efeitos visuais
 * Responsável por criar e gerir todos os efeitos de partículas Niagara do jogo
 */
UCLASS()
class TRANSPERSONALGAME_API UVFX_Manager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Footstep VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayFootstepEffect(const FVector& Location, EDinosaurSpecies Species, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayPlayerFootstepEffect(const FVector& Location, EBiomeType BiomeType);

    // Weather VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartWeatherEffect(EWeatherType WeatherType, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopWeatherEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateWeatherIntensity(float NewIntensity);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayBloodSplatterEffect(const FVector& Location, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayWeaponImpactEffect(const FVector& Location, const FVector& ImpactDirection);

    // Environment VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayCampfireEffect(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopCampfireEffect();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

protected:
    // Dados de configuração VFX por espécie de dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Configuration")
    TMap<EDinosaurSpecies, FVFX_FootstepData> DinosaurFootstepData;

    // Dados de configuração VFX por bioma
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Configuration")
    TMap<EBiomeType, FVFX_FootstepData> BiomeFootstepData;

    // Dados de configuração de tempo
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Configuration")
    FVFX_WeatherData WeatherData;

    // Dados de configuração de combate
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Configuration")
    FVFX_CombatData CombatData;

    // Componentes Niagara activos
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Componente de tempo actual
    UPROPERTY()
    UNiagaraComponent* CurrentWeatherEffect;

    // Componente de fogueira actual
    UPROPERTY()
    UNiagaraComponent* CurrentCampfireEffect;

private:
    // Métodos internos
    void InitializeFootstepData();
    void InitializeWeatherData();
    void InitializeCombatData();

    UNiagaraComponent* SpawnNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);
    void RegisterActiveEffect(UNiagaraComponent* Effect);
    void UnregisterActiveEffect(UNiagaraComponent* Effect);
};