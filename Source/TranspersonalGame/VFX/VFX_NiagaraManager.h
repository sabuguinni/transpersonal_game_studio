#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/PointLight.h"
#include "Particles/ParticleSystemComponent.h"
#include "VFX_NiagaraManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire,
    Dust_Footstep,
    Blood_Impact,
    Water_Splash,
    Smoke_Ambient,
    Rain_Weather,
    Wind_Particles
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Intensity = 1.0f;
        Duration = 5.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_NiagaraManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TArray<FVFX_EffectData> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float LODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float LODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float LODDistance3;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Intensity = 1.0f, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopEffect(EVFX_EffectType EffectType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    float GetDistanceToPlayer() const;

private:
    void CleanupExpiredEffects();
    void CreateFireEffect(const FVFX_EffectData& EffectData);
    void CreateDustEffect(const FVFX_EffectData& EffectData);
    void CreateBloodEffect(const FVFX_EffectData& EffectData);
    void CreateWaterEffect(const FVFX_EffectData& EffectData);

    float LastUpdateTime;
    TArray<UParticleSystemComponent*> ParticleComponents;
    TArray<UPointLightComponent*> LightComponents;
};