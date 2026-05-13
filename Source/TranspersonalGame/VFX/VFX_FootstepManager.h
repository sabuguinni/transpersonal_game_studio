#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_FootstepManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    TSoftObjectPtr<UNiagaraSystem> DustEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float EffectScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float DustIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    FColor DustColor = FColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float EffectDuration = 2.0f;

    FVFX_FootstepData()
    {
        EffectScale = 1.0f;
        DustIntensity = 1.0f;
        DustColor = FColor(139, 69, 19); // Brown dust color
        EffectDuration = 2.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_FootstepManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_FootstepManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Footstep VFX configurations for different dinosaur types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    FVFX_FootstepData TRexFootstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    FVFX_FootstepData RaptorFootstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    FVFX_FootstepData BrachiosaurusFootstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    FVFX_FootstepData GenericFootstep;

    // Main footstep trigger function
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerFootstepVFX(FVector Location, EDinosaurSpecies DinosaurType, float ImpactForce = 1.0f);

    // Specialized footstep functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerTRexFootstep(FVector Location, float ImpactForce = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerRaptorFootstep(FVector Location, float ImpactForce = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerBrachiosaurusFootstep(FVector Location, float ImpactForce = 1.0f);

    // Environmental dust effects
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerEnvironmentalDust(FVector Location, FVector Direction, float Intensity = 1.0f);

    // Screen space effects integration
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerFootstepWithScreenShake(FVector Location, EDinosaurSpecies DinosaurType, float ImpactForce = 1.0f);

private:
    // Internal VFX spawning
    void SpawnFootstepEffect(const FVFX_FootstepData& FootstepData, FVector Location, float ImpactForce);

    // Get footstep data by dinosaur type
    FVFX_FootstepData GetFootstepDataForSpecies(EDinosaurSpecies Species);

    // Active VFX tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveFootstepEffects;

    // Cleanup timer
    FTimerHandle CleanupTimerHandle;

    // Cleanup finished effects
    UFUNCTION()
    void CleanupFinishedEffects();
};