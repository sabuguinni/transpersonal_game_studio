#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "VFX_ParticleManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ParticleEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float IntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy;

    FVFX_ParticleEffectData()
    {
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Duration = 5.0f;
        IntensityMultiplier = 1.0f;
        bAutoDestroy = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactEffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TSoftObjectPtr<UNiagaraSystem> DustEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TSoftObjectPtr<UNiagaraSystem> BloodEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TSoftObjectPtr<UNiagaraSystem> SparkEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float MinImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float MaxParticleCount;

    FVFX_ImpactEffectConfig()
    {
        MinImpactForce = 100.0f;
        MaxParticleCount = 500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ParticleManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Core VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TArray<UNiagaraComponent*> ActiveParticleComponents;

    // VFX Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVFX_ImpactEffectConfig ImpactConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<EBiomeType, FVFX_ParticleEffectData> BiomeAmbientEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float EffectCullDistance;

    // Active Effect Management
    UPROPERTY()
    TArray<FVFX_ParticleEffectData> ActiveEffects;

    UPROPERTY()
    FTimerHandle EffectCleanupTimer;

public:
    // Core VFX Methods
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnParticleEffect(const FVFX_ParticleEffectData& EffectData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnImpactEffect(FVector ImpactLocation, FVector ImpactNormal, float ImpactForce, EImpactType ImpactType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurFootstepEffect(FVector FootLocation, float DinosaurMass, ESurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherEffect(EWeatherType WeatherType, FVector Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFireEffect(FVector FireLocation, float FireIntensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodEffect(FVector BloodLocation, FVector BloodDirection, float BloodAmount);

    // Effect Management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetBiomeAmbientEffect(EBiomeType BiomeType, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void UpdateEffectIntensity(UNiagaraComponent* EffectComponent, float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void StopAllEffects();

    // Utility Methods
    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    bool IsEffectInRange(FVector EffectLocation, float MaxDistance) const;

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    void OptimizeEffectsForPerformance();

protected:
    // Internal Methods
    void InitializeVFXSystems();
    void SetupEffectCleanupTimer();
    UNiagaraComponent* CreateNiagaraComponent(UNiagaraSystem* NiagaraSystem);
    void ConfigureEffectParameters(UNiagaraComponent* Component, const FVFX_ParticleEffectData& EffectData);
    bool ShouldCullEffect(const FVFX_ParticleEffectData& EffectData) const;

public:
    virtual void Tick(float DeltaTime) override;
};