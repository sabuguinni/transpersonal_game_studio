#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "VFX_ParticleSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    Fire_Campfire,
    Dust_Footstep,
    Weather_Rain,
    Combat_Blood,
    Environment_Fog,
    Impact_Rock,
    Water_Splash,
    Wind_Leaves
};

USTRUCT(BlueprintType)
struct FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_ParticleType ParticleType = EVFX_ParticleType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ParticleSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* NiagaraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UStaticMeshComponent* RootMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_ParticleConfig ParticleConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TMap<EVFX_ParticleType, class UNiagaraSystem*> ParticleSystemAssets;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StartParticleEffect(EVFX_ParticleType ParticleType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopParticleEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetParticleIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetParticleScale(FVector NewScale);

    UFUNCTION(BlueprintImplementableEvent, Category = "VFX Events")
    void OnParticleEffectStarted(EVFX_ParticleType ParticleType);

    UFUNCTION(BlueprintImplementableEvent, Category = "VFX Events")
    void OnParticleEffectStopped();

private:
    void InitializeParticleAssets();
    void UpdateParticleParameters();

    float CurrentIntensity;
    bool bIsEffectActive;
    float EffectTimer;
};