#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "VFX_SurvivalEffectsManager.generated.h"

/**
 * VFX Manager for survival gameplay mechanics in Cretaceous period
 * Handles realistic fire effects, environmental particles, and survival-related VFX
 * Focus on scientific accuracy and immersive prehistoric atmosphere
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_SurvivalEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_SurvivalEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CAMPFIRE SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Survival")
    void CreateCampfire(FVector Location, float FireIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Survival")
    void ExtinguishCampfire(class AActor* CampfireActor);

    UFUNCTION(BlueprintCallable, Category = "VFX|Survival")
    void UpdateFireIntensity(class AActor* CampfireActor, float NewIntensity);

    // === ENVIRONMENTAL EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnFootstepDust(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateAtmosphericDust(FVector Location, float Density = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnWindEffect(FVector Location, FVector WindDirection, float Strength = 1.0f);

    // === COOKING & CRAFTING VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Survival")
    void SpawnCookingSmoke(FVector Location, float CookingProgress = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Survival")
    void CreateToolCraftingSparks(FVector Location, float CraftingIntensity = 1.0f);

protected:
    // === NIAGARA SYSTEMS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Fire")
    class UNiagaraSystem* CampfireSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Fire")
    class UNiagaraSystem* CookingSmokeSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Environment")
    class UNiagaraSystem* FootstepDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Environment")
    class UNiagaraSystem* AtmosphericDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Environment")
    class UNiagaraSystem* WindParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Crafting")
    class UNiagaraSystem* CraftingSparksSystem;

    // === AUDIO INTEGRATION ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Audio")
    class USoundCue* CampfireCrackleSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Audio")
    class USoundCue* CookingSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Audio")
    class USoundCue* CraftingSound;

    // === LIGHTING EFFECTS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Lighting")
    float CampfireLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Lighting")
    FLinearColor CampfireLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Lighting")
    float CampfireLightRadius;

    // === PERFORMANCE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance")
    float MaxEffectDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance")
    int32 MaxActiveFireEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance")
    int32 MaxActiveEnvironmentalEffects;

private:
    // === INTERNAL STATE ===
    UPROPERTY()
    TArray<class AActor*> ActiveCampfires;

    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveEnvironmentalEffects;

    // === HELPER FUNCTIONS ===
    void CleanupInactiveEffects();
    bool IsPlayerNearby(FVector Location, float Distance) const;
    void UpdateEffectLOD(class UNiagaraComponent* Effect, float DistanceToPlayer);

    // === PERFORMANCE TRACKING ===
    float LastCleanupTime;
    static constexpr float CleanupInterval = 5.0f;
};