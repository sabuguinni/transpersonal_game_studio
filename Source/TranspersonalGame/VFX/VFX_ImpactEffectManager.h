#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_ImpactEffectManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TSoftObjectPtr<UNiagaraSystem> DustEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TSoftObjectPtr<UNiagaraSystem> BloodEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TSoftObjectPtr<UNiagaraSystem> WaterSplashEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float EffectScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float Duration;

    FVFX_ImpactData()
    {
        EffectScale = 1.0f;
        Duration = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ImpactEffectManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactEffectManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_ImpactData ImpactSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectPoolingRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDustImpact(const FVector& Location, const FVector& Normal, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(const FVector& Location, const FVector& Direction, float Amount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWaterSplash(const FVector& Location, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    void SpawnNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation, float Scale);
    void RemoveOldestEffect();
};