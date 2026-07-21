#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "ArchRuinCluster.generated.h"

/**
 * Cretaceous-era ruin cluster actor.
 * Represents a group of ancient stone pillars and debris
 * scattered across the prehistoric landscape.
 * Agent #7 — Architecture & Interior Agent
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchRuinCluster : public AActor
{
	GENERATED_BODY()

public:
	AArchRuinCluster();

	virtual void BeginPlay() override;

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
	USceneComponent* RootScene;

	/** Primary standing pillar mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
	UStaticMeshComponent* PillarMesh;

	/** Secondary broken pillar */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
	UStaticMeshComponent* BrokenPillarMesh;

	/** Stone slab base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
	UStaticMeshComponent* SlabMesh;

	/** Atmospheric point light (amber, warm prehistoric tone) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture|Lighting")
	UPointLightComponent* AmbientLight;

	/** Whether this ruin emits ambient light */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Lighting")
	bool bEnableAmbientLight;

	/** Light intensity for ambient ruin glow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Lighting", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
	float AmbientLightIntensity;

	/** Ruin age in years (affects weathering visual parameters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Lore")
	float RuinAgeYears;

	/** Whether this ruin is a discoverable landmark */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Gameplay")
	bool bIsLandmark;

	/** Landmark discovery radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Gameplay", meta = (ClampMin = "100.0"))
	float LandmarkDiscoveryRadius;

	/** Apply weathering scale based on ruin age */
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void ApplyWeathering();

	/** Get the world-space center of this ruin cluster */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture")
	FVector GetClusterCenter() const;
};
