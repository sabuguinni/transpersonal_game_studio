#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFXParticleManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ParticleEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UParticleSystem* ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy;

    FVFX_ParticleEffectData()
    {
        ParticleSystem = nullptr;
        Duration = 3.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        bAutoDestroy = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_FootstepEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVFX_ParticleEffectData DustCloud;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVFX_ParticleEffectData GroundCracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVFX_ParticleEffectData DebrisScatter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float MinImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float MaxEffectRadius;

    FVFX_FootstepEffectData()
    {
        MinImpactForce = 100.0f;
        MaxEffectRadius = 500.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ParticleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Footstep VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepEffect(const FVector& Location, float ImpactForce, EBiomeType BiomeType);

    // Blood VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodEffect(const FVector& Location, const FVector& ImpactDirection, float DamageAmount);

    // Fire VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopCampfireEffect(const FVector& Location);

    // Weather VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartRainEffect(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopRainEffect();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

protected:
    // Footstep effect configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footsteps")
    TMap<EBiomeType, FVFX_FootstepEffectData> FootstepEffects;

    // Blood effect configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Blood")
    FVFX_ParticleEffectData BloodSplatter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Blood")
    FVFX_ParticleEffectData BloodDrops;

    // Fire effect configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Fire")
    FVFX_ParticleEffectData CampfireFlames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Fire")
    FVFX_ParticleEffectData CampfireSmoke;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Fire")
    FVFX_ParticleEffectData CampfireEmbers;

    // Weather effect configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    FVFX_ParticleEffectData RainDrops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    FVFX_ParticleEffectData RainSplash;

    // Active effects tracking
    UPROPERTY()
    TArray<UParticleSystemComponent*> ActiveEffects;

    UPROPERTY()
    TArray<UParticleSystemComponent*> CampfireEffects;

    UPROPERTY()
    UParticleSystemComponent* RainEffect;

private:
    // Helper functions
    UParticleSystemComponent* SpawnParticleEffect(const FVFX_ParticleEffectData& EffectData, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);
    void InitializeDefaultEffects();
    FVector CalculateEffectScale(float ImpactForce, float MinForce, float MaxForce) const;
    void RemoveExpiredEffect(UParticleSystemComponent* Effect);
};