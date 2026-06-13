#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "VFX_SystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire,
    Dust,
    Blood,
    Rain,
    Fog,
    Ash,
    Insects,
    Impact,
    Explosion
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire;
        Duration = 5.0f;
        Intensity = 1.0f;
        bLooping = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_SystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TArray<FVFX_EffectData> AvailableEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFireEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDustEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherEffect(EVFX_EffectType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ClearAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetGlobalVFXIntensity(float NewIntensity);

private:
    UPROPERTY()
    float GlobalVFXIntensity;

    UPROPERTY()
    TMap<EVFX_EffectType, TSoftObjectPtr<UNiagaraSystem>> EffectSystemMap;

    void InitializeEffectSystems();
    void CleanupExpiredEffects();
    UNiagaraComponent* CreateNiagaraComponent(UNiagaraSystem* System, FVector Location, FRotator Rotation, float Scale);

    FTimerHandle CleanupTimerHandle;
};