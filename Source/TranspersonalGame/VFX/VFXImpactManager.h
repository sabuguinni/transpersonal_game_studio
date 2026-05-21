#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "SharedTypes.h"
#include "VFXImpactManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float ImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    EDinosaurSpecies DinosaurType;

    FVFX_ImpactData()
    {
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactForce = 1.0f;
        DinosaurType = EDinosaurSpecies::TRex;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    UParticleSystemComponent* DustParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    UParticleSystemComponent* BloodParticleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float FootstepDustIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float BloodSplatterScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EnvironmentalDustRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerFootstepImpact(const FVFX_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerBloodSplatter(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerEnvironmentalDust(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SetDinosaurFootstepScale(EDinosaurSpecies Species, float Scale);

private:
    void InitializeParticleSystems();
    void ConfigureFootstepVFX(EDinosaurSpecies Species);
    
    UPROPERTY()
    TMap<EDinosaurSpecies, float> FootstepScaleMap;
};