#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "VFX_ParticleManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Dust_Footstep       UMETA(DisplayName = "Footstep Dust"),
    Blood_Impact        UMETA(DisplayName = "Blood Splatter"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Smoke_General       UMETA(DisplayName = "General Smoke"),
    Ash_Volcanic        UMETA(DisplayName = "Volcanic Ash"),
    Mist_Forest         UMETA(DisplayName = "Forest Mist"),
    Sparks_Tool         UMETA(DisplayName = "Tool Sparks")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_ParticleType ParticleType = EVFX_ParticleType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float SpawnRate = 50.0f;

    FVFX_ParticleConfig()
    {
        ParticleType = EVFX_ParticleType::Fire_Campfire;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 5.0f;
        bAutoDestroy = true;
        SpawnRate = 50.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ParticleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Particle Manager")
    TArray<FVFX_ParticleConfig> ParticleConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Particle Manager")
    bool bEnableVFX = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Particle Manager")
    float GlobalVFXScale = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Particle Manager")
    TArray<UNiagaraComponent*> ActiveParticles;

    UFUNCTION(BlueprintCallable, Category = "VFX Particle Manager")
    UNiagaraComponent* SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX Particle Manager")
    void SpawnFootstepDust(FVector FootLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Particle Manager")
    void SpawnCampfire(FVector FireLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX Particle Manager")
    void SpawnBloodSplatter(FVector ImpactLocation, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX Particle Manager")
    void SpawnWaterSplash(FVector WaterLocation, float SplashIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Particle Manager")
    void StopAllParticles();

    UFUNCTION(BlueprintCallable, Category = "VFX Particle Manager")
    void CleanupExpiredParticles();

private:
    FVFX_ParticleConfig* GetParticleConfig(EVFX_ParticleType ParticleType);
    
    void InitializeDefaultConfigs();
    
    float LastCleanupTime = 0.0f;
    const float CleanupInterval = 2.0f;
};