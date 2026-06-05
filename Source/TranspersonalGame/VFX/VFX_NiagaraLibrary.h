#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "VFX_NiagaraLibrary.generated.h"

/**
 * VFX Niagara Library - Centralized VFX system for prehistoric survival game
 * Manages particle effects for campfires, combat impacts, weather, and dinosaur interactions
 * All effects are realistic and physically plausible for prehistoric setting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CAMPFIRE VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Fire")
    void SpawnCampfireEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Fire")
    void StopCampfireEffect();

    // === COMBAT VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodImpactEffect(FVector Location, FVector Normal);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnWeaponImpactEffect(FVector Location, FVector Normal, float Force = 1.0f);

    // === DINOSAUR VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnFootstepDustEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnDinosaurBreathEffect(FVector Location, FVector Direction);

    // === WEATHER VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SpawnRainEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SpawnDustStormEffect(FVector Location, FVector Direction, float Intensity = 1.0f);

    // === ENVIRONMENT VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnWaterfallSprayEffect(FVector Location, float Height = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnVolcanicAshEffect(FVector Location, float Radius = 1000.0f);

protected:
    // Niagara System References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    TSoftObjectPtr<UNiagaraSystem> CampfireSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    TSoftObjectPtr<UNiagaraSystem> BloodImpactSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    TSoftObjectPtr<UNiagaraSystem> FootstepDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    TSoftObjectPtr<UNiagaraSystem> RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    TSoftObjectPtr<UNiagaraSystem> DustStormSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    TSoftObjectPtr<UNiagaraSystem> WaterfallSystem;

    // Active Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Components")
    TArray<UNiagaraComponent*> ActiveEffects;

    // VFX Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float GlobalVFXScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    bool bEnableVFXLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float MaxVFXDistance;

private:
    // Helper functions
    UNiagaraComponent* CreateNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation = FRotator::ZeroRotator);
    void CleanupFinishedEffects();
    bool IsPlayerNearby(FVector Location, float MaxDistance) const;
};