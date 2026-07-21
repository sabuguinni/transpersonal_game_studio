#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/TriggerBox.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "VFX_FootstepManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float ImpactForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DustIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bCreateCrack = false;

    FVFX_FootstepData()
    {
        ImpactLocation = FVector::ZeroVector;
        ImpactForce = 1000.0f;
        SurfaceType = EVFX_SurfaceType::Dirt;
        DustIntensity = 1.0f;
        bCreateCrack = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* DustEffect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* RockDebrisEffect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* WaterSplashEffect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectLifetime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 MaxActiveEffects = 50;

    FVFX_EffectSettings()
    {
        DustEffect = nullptr;
        RockDebrisEffect = nullptr;
        WaterSplashEffect = nullptr;
        EffectLifetime = 3.0f;
        MaxActiveEffects = 50;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_FootstepManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_FootstepManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_EffectSettings EffectSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MinImpactForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxEffectDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableScreenShake = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float ScreenShakeIntensity = 0.5f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<class UNiagaraComponent*> ActiveEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    int32 EffectCount = 0;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateFootstepEffect(const FVFX_FootstepData& FootstepData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateDustCloud(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateRockDebris(FVector Location, float Force = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateWaterSplash(FVector Location, float Radius = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupOldEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectQuality(int32 QualityLevel);

    UFUNCTION(BlueprintPure, Category = "VFX")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerScreenShake(FVector Location, float Intensity);

private:
    void InitializeEffectSystems();
    void LoadDefaultEffects();
    UNiagaraComponent* SpawnNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation = FRotator::ZeroRotator);
    void RemoveExpiredEffects();

    UPROPERTY()
    TArray<float> EffectTimestamps;

    float LastCleanupTime = 0.0f;
    const float CleanupInterval = 1.0f;
};