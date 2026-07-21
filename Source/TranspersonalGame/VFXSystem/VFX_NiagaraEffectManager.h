#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "VFX_NiagaraEffectManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float SpawnCooldown = 1.0f;

    FVFX_EffectSettings()
    {
        Scale = FVector(1.0f);
        Duration = 5.0f;
        bAutoDestroy = true;
        SpawnCooldown = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_BiomeEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    FVFX_EffectSettings AmbientParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    FVFX_EffectSettings WeatherEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    FVFX_EffectSettings EnvironmentalDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float EffectIntensity = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraEffectManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UVFX_NiagaraEffectManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    UNiagaraComponent* SpawnVFXAtLocation(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    UNiagaraComponent* SpawnVFXAttached(const FString& EffectName, USceneComponent* AttachComponent, const FVector& RelativeLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void SpawnCampfireVFX(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void SpawnDinosaurFootstepVFX(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void SpawnBloodImpactVFX(const FVector& Location, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void SpawnWeatherVFX(EBiomeType BiomeType, const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void SetBiomeVFXIntensity(EBiomeType BiomeType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CleanupExpiredEffects();

protected:
    // VFX effect registry
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Registry")
    TMap<FString, FVFX_EffectSettings> EffectRegistry;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Registry")
    TMap<EBiomeType, FVFX_BiomeEffects> BiomeEffects;

    // Active effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    TMap<FString, float> EffectCooldowns;

private:
    void InitializeEffectRegistry();
    void InitializeBiomeEffects();
    bool CanSpawnEffect(const FString& EffectName);
    void RegisterEffectCooldown(const FString& EffectName);
    UNiagaraSystem* LoadNiagaraSystem(const FString& AssetPath);

    FTimerHandle CleanupTimerHandle;
    static constexpr float CleanupInterval = 10.0f;
};