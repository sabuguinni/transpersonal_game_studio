#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMeshActor.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None UMETA(DisplayName = "None"),
    DinosaurFootstep UMETA(DisplayName = "Dinosaur Footstep"),
    DinosaurBreath UMETA(DisplayName = "Dinosaur Breath"),
    BloodSplatter UMETA(DisplayName = "Blood Splatter"),
    DustCloud UMETA(DisplayName = "Dust Cloud"),
    WaterSplash UMETA(DisplayName = "Water Splash"),
    CampfireSmoke UMETA(DisplayName = "Campfire Smoke"),
    RainDroplets UMETA(DisplayName = "Rain Droplets"),
    WeaponImpact UMETA(DisplayName = "Weapon Impact")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Duration = 3.0f;
        Intensity = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_Manager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_Manager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<EVFX_EffectType, UParticleSystem*> EffectLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TArray<UParticleSystemComponent*> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoCleanupEffects;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UParticleSystemComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(FVector Location, FVector Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDustCloud(FVector Location, float Radius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWaterSplash(FVector Location, float Force = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireSmoke(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupFinishedEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

private:
    void InitializeEffectLibrary();
    void RegisterEffectSystem(EVFX_EffectType EffectType, const FString& ParticleSystemPath);
};

#include "VFXManager.generated.h"