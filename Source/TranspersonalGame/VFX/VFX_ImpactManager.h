#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_ImpactManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float ImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    EDinosaurSpecies DinosaurSpecies;

    FVFX_ImpactData()
    {
        ImpactLocation = FVector::ZeroVector;
        ImpactForce = 1.0f;
        BiomeType = EBiomeType::Savanna;
        DinosaurSpecies = EDinosaurSpecies::TRex;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* DustParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* DebrisParticleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float BaseImpactRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxParticleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoTriggerOnSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    EBiomeType CurrentBiome;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerFootstepImpact(const FVFX_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDinosaurImpact(EDinosaurSpecies Species, FVector Location, float Force);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SetBiomeType(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void StopAllEffects();

protected:
    UFUNCTION()
    void ConfigureParticlesForBiome();

    UFUNCTION()
    void ScaleEffectForDinosaur(EDinosaurSpecies Species);

private:
    void InitializeParticleComponents();
    void SetupBiomeSpecificMaterials();
};