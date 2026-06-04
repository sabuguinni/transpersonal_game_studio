#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "SharedTypes.h"
#include "VFX_EffectManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        NiagaraSystem = nullptr;
        Scale = FVector(1.0f);
        Duration = 2.0f;
        bAutoDestroy = true;
    }
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    DinosaurFootstep = 1,
    BloodImpact = 2,
    CampfireFire = 3,
    WeatherRain = 4,
    EnvironmentDust = 5,
    CombatSparks = 6,
    WaterSplash = 7,
    MAX = 8
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_EffectManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EffectManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalEffectScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects = 50;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, float ScaleMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepEffect(const FVector& Location, float CreatureSize = 1.0f, ESurfaceType SurfaceType = ESurfaceType::Grass);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodEffect(const FVector& Location, const FVector& ImpactNormal, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEnvironmentalEffect(EVFX_EffectType EffectType, const FVector& Location, float Duration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

protected:
    void InitializeEffectDatabase();
    void RemoveNullEffects();
    FVector CalculateEffectScale(EVFX_EffectType EffectType, float ScaleMultiplier) const;
};