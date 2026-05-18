#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    Blood           UMETA(DisplayName = "Blood Splatter"),
    Dust            UMETA(DisplayName = "Dust Cloud"),
    Fire            UMETA(DisplayName = "Fire"),
    Water           UMETA(DisplayName = "Water Splash"),
    Footstep        UMETA(DisplayName = "Footstep Impact"),
    Weather         UMETA(DisplayName = "Weather Effect"),
    Combat          UMETA(DisplayName = "Combat Impact")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        Scale = FVector(1.0f);
        Duration = 2.0f;
        bAutoDestroy = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFXEffect(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(const FVector& Location, const FVector& Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDustCloud(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepDust(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfire(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterVFXEffect(EVFX_EffectType EffectType, UNiagaraSystem* NiagaraSystem);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<EVFX_EffectType, FVFX_EffectData> VFXDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 MaxActiveEffects = 50;

    void InitializeDefaultVFX();
    UNiagaraComponent* CreateNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation);
};