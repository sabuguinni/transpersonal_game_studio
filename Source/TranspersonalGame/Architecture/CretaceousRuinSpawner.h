#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "CretaceousRuinSpawner.generated.h"

/**
 * Spawns procedural Cretaceous-era stone ruin structures in the world.
 * Handles pillar circles, altar platforms, and interior shelter scenes.
 * Agent #7 — Architecture & Interior Agent
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousRuinSpawner : public AActor
{
	GENERATED_BODY()

public:
	ACretaceousRuinSpawner();

	virtual void BeginPlay() override;

	/** Spawn the full ruin complex at this actor's location */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
	void SpawnRuinComplex();

	/** Spawn a single stone pillar at the given offset from this actor */
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	AActor* SpawnPillar(FVector Offset, float HeightScale);

	/** Spawn a central altar platform */
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	AActor* SpawnAltarPlatform();

	/** Spawn a primitive interior shelter with fire pit */
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void SpawnInteriorShelter(FVector ShelterOffset);

	/** Number of pillars in the ruin circle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Config")
	int32 PillarCount = 8;

	/** Radius of the pillar circle in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Config")
	float PillarCircleRadius = 500.0f;

	/** Minimum pillar height scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Config")
	float MinPillarHeight = 2.0f;

	/** Maximum pillar height scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Config")
	float MaxPillarHeight = 5.0f;

	/** Whether to spawn an interior shelter alongside the ruin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Config")
	bool bSpawnInterior = true;

	/** Offset from ruin center where interior shelter is placed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Config")
	FVector InteriorOffset = FVector(1000.0f, 0.0f, 0.0f);

	/** Fire pit light intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Lighting")
	float FireLightIntensity = 3000.0f;

	/** Fire pit light color (warm orange) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Lighting")
	FLinearColor FireLightColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);

private:
	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
	USceneComponent* RuinRoot;

	/** All spawned actors in this ruin complex */
	UPROPERTY()
	TArray<AActor*> SpawnedActors;
};
