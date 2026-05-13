#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_ParticleSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EnvironmentalType : uint8
{
    DustCloud       UMETA(DisplayName = "Dust Cloud"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    FootstepImpact  UMETA(DisplayName = "Footstep Impact"),
    RainDroplets    UMETA(DisplayName = "Rain Droplets"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    WindParticles   UMETA(DisplayName = "Wind Particles")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EnvironmentalType EffectType = EVFX_EnvironmentalType::DustCloud;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor Color = FLinearColor::White;

    FVFX_EffectData()
    {
        EffectType = EVFX_EnvironmentalType::DustCloud;
        Duration = 2.0f;
        Intensity = 1.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Color = FLinearColor::White;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ParticleSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Environmental VFX Methods
    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnDustCloud(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental") 
    void SpawnWaterSplash(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnFootstepImpact(const FVector& Location, const FVector& ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnRainEffect(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnVolcanicAsh(const FVector& Location, const FVector& WindDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnWindParticles(const FVector& Location, const FVector& WindDirection, float Strength = 1.0f);

    // Generic effect spawner
    UFUNCTION(BlueprintCallable, Category = "VFX|Core")
    void SpawnEnvironmentalEffect(const FVFX_EffectData& EffectData, const FVector& Location);

    // Effect management
    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void SetGlobalVFXIntensity(float NewIntensity);

protected:
    // Niagara system references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    class UNiagaraSystem* DustCloudSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    class UNiagaraSystem* WaterSplashSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    class UNiagaraSystem* FootstepSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    class UNiagaraSystem* RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    class UNiagaraSystem* VolcanicAshSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    class UNiagaraSystem* WindParticleSystem;

    // Active effect components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Components")
    TArray<class UNiagaraComponent*> ActiveEffects;

    // VFX settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float GlobalIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    bool bEnableVFXLOD = true;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float LODDistanceNear = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float LODDistanceFar = 5000.0f;

private:
    // Internal helper methods
    void CleanupExpiredEffects();
    float CalculateLODScale(const FVector& EffectLocation) const;
    bool ShouldSpawnEffect(const FVector& Location) const;
    void InitializeNiagaraSystems();
};