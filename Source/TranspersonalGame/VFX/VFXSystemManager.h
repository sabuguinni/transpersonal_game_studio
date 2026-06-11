#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire,
    Dust,
    Blood,
    Water,
    Weather,
    Impact,
    Footstep,
    Breath
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraSystem* ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* Sound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale;

    FVFX_EffectData()
    {
        ParticleSystem = nullptr;
        Sound = nullptr;
        Duration = 3.0f;
        Scale = 1.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UVFX_SystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepEffect(FVector Location, bool bIsLarge = false);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnImpactEffect(FVector Location, FVector Normal);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFireEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodEffect(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Radius = 1000.0f);

protected:
    UPROPERTY()
    TMap<EVFX_EffectType, FVFX_EffectData> EffectLibrary;

    void InitializeEffectLibrary();
    UNiagaraComponent* SpawnNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation, float Scale);
    UAudioComponent* PlayEffectSound(USoundCue* Sound, FVector Location);
};

#include "VFXSystemManager.generated.h"