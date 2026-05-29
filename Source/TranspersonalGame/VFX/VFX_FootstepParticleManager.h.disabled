#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "VFX_FootstepParticleManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_FootstepType : uint8
{
    Light       UMETA(DisplayName = "Light Step"),
    Medium      UMETA(DisplayName = "Medium Step"),
    Heavy       UMETA(DisplayName = "Heavy Step"),
    Massive     UMETA(DisplayName = "Massive Step")
};

UENUM(BlueprintType)
enum class EVFX_SurfaceType : uint8
{
    Dirt        UMETA(DisplayName = "Dirt"),
    Mud         UMETA(DisplayName = "Mud"),
    Rock        UMETA(DisplayName = "Rock"),
    Sand        UMETA(DisplayName = "Sand"),
    Grass       UMETA(DisplayName = "Grass")
};

USTRUCT(BlueprintType)
struct FVFX_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_FootstepType FootstepType = EVFX_FootstepType::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float ParticleIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DustRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor DustColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);

    FVFX_FootstepConfig()
    {
        FootstepType = EVFX_FootstepType::Medium;
        SurfaceType = EVFX_SurfaceType::Dirt;
        ParticleIntensity = 1.0f;
        DustRadius = 50.0f;
        DustColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_FootstepParticleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_FootstepParticleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerFootstepEffect(const FVector& Location, const FVFX_FootstepConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerDinosaurFootstep(const FVector& Location, EVFX_FootstepType StepType, EVFX_SurfaceType Surface);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetFootstepIntensityMultiplier(float Multiplier);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    TSoftObjectPtr<UNiagaraSystem> DustParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    TSoftObjectPtr<UNiagaraSystem> MudSplashSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    TSoftObjectPtr<UNiagaraSystem> RockDebrisSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float GlobalIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float MaxFootstepDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bEnableDistanceAttenuation = true;

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveParticleComponents;

    void SpawnParticleEffect(const FVector& Location, UNiagaraSystem* System, const FVFX_FootstepConfig& Config);
    float CalculateDistanceAttenuation(const FVector& EffectLocation) const;
    UNiagaraSystem* GetSystemForSurface(EVFX_SurfaceType SurfaceType) const;
    void CleanupFinishedEffects();
};