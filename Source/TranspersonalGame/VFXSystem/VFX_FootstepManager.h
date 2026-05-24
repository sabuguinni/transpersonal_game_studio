#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TranspersonalGame/SharedTypes.h"
#include "VFX_FootstepManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float DustIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float ParticleSize = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float DinosaurMass = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    EDinosaurSpecies Species = EDinosaurSpecies::TRex;

    FVFX_FootstepData()
    {
        DustIntensity = 1.0f;
        ParticleSize = 100.0f;
        ImpactLocation = FVector::ZeroVector;
        DinosaurMass = 1000.0f;
        Species = EDinosaurSpecies::TRex;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_FootstepManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_FootstepManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    class UNiagaraComponent* DustParticleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    class UNiagaraSystem* FootstepDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxFootstepRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float DustLifetime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Presets")
    TMap<EDinosaurSpecies, FVFX_FootstepData> SpeciesPresets;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerFootstepVFX(const FVector& Location, EDinosaurSpecies Species, float DinosaurMass);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateDustCloud(const FVector& Location, float Intensity, float Size);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetupSpeciesPresets();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FVFX_FootstepData GetFootstepDataForSpecies(EDinosaurSpecies Species) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    float CalculateDistanceIntensity(const FVector& ImpactLocation) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

private:
    TArray<class UNiagaraComponent*> ActiveDustEffects;
    TArray<float> EffectTimestamps;

    void InitializeNiagaraSystem();
    void UpdateActiveEffects(float DeltaTime);
};