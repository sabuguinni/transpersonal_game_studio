#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "VFXSystemManager.generated.h"

/**
 * VFX System Manager - Gere todos os efeitos visuais do jogo
 * Responsável por footsteps de dinossauros, impactos de combate, efeitos ambientais
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_SystemManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UVFX_SystemManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// === FOOTSTEP VFX ===
	UFUNCTION(BlueprintCallable, Category = "VFX|Footsteps")
	void SpawnFootstepEffect(FVector Location, EDinosaurSpecies DinoType, float ImpactForce);

	UFUNCTION(BlueprintCallable, Category = "VFX|Footsteps")
	void SpawnPlayerFootstepEffect(FVector Location, ESurfaceType SurfaceType);

	// === COMBAT VFX ===
	UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
	void SpawnBloodImpactEffect(FVector Location, FVector ImpactDirection, float Damage);

	UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
	void SpawnWeaponImpactEffect(FVector Location, EWeaponType WeaponType, ESurfaceType SurfaceHit);

	// === ENVIRONMENTAL VFX ===
	UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
	void SpawnDustCloudEffect(FVector Location, float Intensity);

	UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
	void SpawnDebrisEffect(FVector Location, FVector Direction, int32 DebrisCount);

	// === VFX MANAGEMENT ===
	UFUNCTION(BlueprintCallable, Category = "VFX|System")
	void SetVFXQualityLevel(int32 QualityLevel);

	UFUNCTION(BlueprintCallable, Category = "VFX|System")
	void CleanupOldEffects();

protected:
	// === NIAGARA SYSTEMS ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
	UNiagaraSystem* FootstepDustSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
	UNiagaraSystem* BloodImpactSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
	UNiagaraSystem* WeaponImpactSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
	UNiagaraSystem* DebrisSystem;

	// === VFX SETTINGS ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Settings")
	int32 MaxActiveEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Settings")
	float EffectLifetime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Settings")
	bool bEnableVFXLOD;

	// === ACTIVE EFFECTS TRACKING ===
	UPROPERTY()
	TArray<UNiagaraComponent*> ActiveEffects;

	UPROPERTY()
	float LastCleanupTime;

private:
	// === INTERNAL METHODS ===
	UNiagaraSystem* GetFootstepSystemForSpecies(EDinosaurSpecies Species);
	UNiagaraSystem* GetImpactSystemForWeapon(EWeaponType WeaponType);
	void RegisterActiveEffect(UNiagaraComponent* Effect);
	bool ShouldSpawnEffect(FVector Location);
	float CalculateEffectIntensity(float Distance);
};