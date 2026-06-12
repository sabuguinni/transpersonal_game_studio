#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "VFX_ParticleManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Dino_Footstep      UMETA(DisplayName = "Dinosaur Footstep"),
    Weather_Rain       UMETA(DisplayName = "Rain"),
    Combat_Blood       UMETA(DisplayName = "Blood Impact"),
    Environment_Dust   UMETA(DisplayName = "Dust Cloud"),
    Water_Splash       UMETA(DisplayName = "Water Splash")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UParticleSystem> LegacyParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Duration = 5.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ParticleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    TArray<FVFX_EffectData> EffectDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bUseNiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalEffectScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffectAtActor(EVFX_EffectType EffectType, AActor* TargetActor, FVector Offset = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectQuality(int32 QualityLevel);

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveNiagaraComponents;

    UPROPERTY()
    TArray<UParticleSystemComponent*> ActiveParticleComponents;

    void CleanupFinishedEffects();
    FVFX_EffectData* GetEffectData(EVFX_EffectType EffectType);
    void SpawnNiagaraEffect(const FVFX_EffectData& EffectData, FVector Location, FRotator Rotation, float Scale);
    void SpawnLegacyEffect(const FVFX_EffectData& EffectData, FVector Location, FRotator Rotation, float Scale);
};