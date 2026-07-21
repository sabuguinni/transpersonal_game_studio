#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "VFXBloodManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_BloodIntensity : uint8
{
    Light       UMETA(DisplayName = "Light Bleeding"),
    Medium      UMETA(DisplayName = "Medium Bleeding"), 
    Heavy       UMETA(DisplayName = "Heavy Bleeding"),
    Arterial    UMETA(DisplayName = "Arterial Spray")
};

USTRUCT(BlueprintType)
struct FVFX_BloodEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    EVFX_BloodIntensity Intensity = EVFX_BloodIntensity::Light;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    float BloodAmount = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    bool bCreateDecal = true;

    FVFX_BloodEffectData()
    {
        Intensity = EVFX_BloodIntensity::Light;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        BloodAmount = 1.0f;
        bCreateDecal = true;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_BloodManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_BloodManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood VFX")
    class UParticleSystem* LightBloodParticles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood VFX")
    class UParticleSystem* MediumBloodParticles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood VFX")
    class UParticleSystem* HeavyBloodParticles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood VFX")
    class UParticleSystem* ArterialBloodParticles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood VFX")
    class UMaterialInterface* BloodDecalMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    float DecalLifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    FVector DecalSize = FVector(50.0f, 50.0f, 50.0f);

public:
    UFUNCTION(BlueprintCallable, Category = "Blood VFX")
    void SpawnBloodEffect(const FVFX_BloodEffectData& BloodData);

    UFUNCTION(BlueprintCallable, Category = "Blood VFX")
    void SpawnBloodSplatter(FVector Location, FVector Normal, EVFX_BloodIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "Blood VFX")
    void CreateBloodDecal(FVector Location, FVector Normal, float Size = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Blood VFX")
    void CleanupBloodEffects();

private:
    UPROPERTY()
    TArray<class UParticleSystemComponent*> ActiveBloodParticles;

    UPROPERTY()
    TArray<class UDecalComponent*> ActiveBloodDecals;

    UParticleSystem* GetParticleSystemForIntensity(EVFX_BloodIntensity Intensity);
    void SpawnBloodParticles(FVector Location, FVector Normal, UParticleSystem* ParticleSystem);
};

#include "VFXBloodManager.generated.h"