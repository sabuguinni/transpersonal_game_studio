#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_ImpactManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    TSoftObjectPtr<UNiagaraSystem> DustEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    TSoftObjectPtr<UNiagaraSystem> BloodEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    TSoftObjectPtr<UNiagaraSystem> WaterSplashEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float EffectScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float EffectDuration;

    FVFX_ImpactData()
    {
        EffectScale = 1.0f;
        EffectDuration = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EnvironmentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    TSoftObjectPtr<UNiagaraSystem> CampfireEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    TSoftObjectPtr<UNiagaraSystem> RainEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    TSoftObjectPtr<UNiagaraSystem> FogEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Environment")
    TSoftObjectPtr<UNiagaraSystem> VolcanicAshEffect;

    FVFX_EnvironmentData()
    {
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_ImpactData ImpactSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_EnvironmentData EnvironmentSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableVFXLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float VFXCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects;

    TArray<UNiagaraComponent*> ActiveEffects;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnDustImpact(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnBloodImpact(const FVector& Location, const FVector& Direction, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnWaterSplash(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnCampfire(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnRainEffect(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnVolcanicAsh(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetVFXQuality(int32 QualityLevel);

private:
    void InitializeVFXAssets();
    UNiagaraComponent* CreateNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);
    bool IsWithinCullingDistance(const FVector& Location) const;
    void ApplyLODSettings(UNiagaraComponent* Effect, float Distance) const;
};