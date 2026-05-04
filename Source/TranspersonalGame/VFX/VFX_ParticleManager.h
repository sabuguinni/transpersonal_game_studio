#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraActor.h"
#include "SharedTypes.h"
#include "VFX_ParticleManager.generated.h"

/**
 * Sistema de gestão de efeitos visuais para o jogo pré-histórico
 * Gere partículas Niagara para pegadas, impactos, fogo e ambiente
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ParticleManager : public AActor
{
	GENERATED_BODY()

public:
	AVFX_ParticleManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Sistemas Niagara para diferentes efeitos
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
	class UNiagaraSystem* FootstepDustSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
	class UNiagaraSystem* CampfireSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
	class UNiagaraSystem* BloodImpactSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
	class UNiagaraSystem* WeatherRainSystem;

	// Pool de componentes Niagara reutilizáveis
	UPROPERTY()
	TArray<class UNiagaraComponent*> ParticlePool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	int32 MaxPoolSize = 50;

public:
	// Métodos para spawnar efeitos específicos
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnCampfireEffect(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnBloodImpact(FVector Location, FVector ImpactDirection);

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnWeatherEffect(EBiomeType BiomeType, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void StopAllEffects();

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetEffectIntensity(float Intensity);

private:
	// Gestão do pool de partículas
	class UNiagaraComponent* GetPooledParticle();
	void ReturnParticleToPool(class UNiagaraComponent* Particle);

	// Configuração de efeitos por bioma
	void ConfigureEffectForBiome(class UNiagaraComponent* Effect, EBiomeType BiomeType);

	UPROPERTY()
	float CurrentEffectIntensity = 1.0f;
};