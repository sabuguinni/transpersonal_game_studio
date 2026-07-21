#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_TerrainInteraction.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
	Grass = 0,
	Dirt = 1,
	Rock = 2,
	Sand = 3,
	Mud = 4,
	Snow = 5,
	Water = 6,
	Lava = 7
};

USTRUCT(BlueprintType)
struct FCore_TerrainProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	ECore_TerrainType TerrainType = ECore_TerrainType::Grass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float FrictionMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float BounceMultiplier = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float MovementSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	bool bCanDigTerrain = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	bool bLeavesFootprints = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float StaminaDrainMultiplier = 1.0f;

	FCore_TerrainProperties()
	{
		TerrainType = ECore_TerrainType::Grass;
		FrictionMultiplier = 1.0f;
		BounceMultiplier = 0.3f;
		MovementSpeedMultiplier = 1.0f;
		bCanDigTerrain = false;
		bLeavesFootprints = true;
		StaminaDrainMultiplier = 1.0f;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainInteraction : public UActorComponent
{
	GENERATED_BODY()

public:
	UCore_TerrainInteraction();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Terrain Detection
	UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
	ECore_TerrainType DetectTerrainTypeAtLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
	FCore_TerrainProperties GetTerrainPropertiesAtLocation(const FVector& Location);

	// Terrain Modification
	UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
	bool DigTerrainAtLocation(const FVector& Location, float Radius, float Depth);

	UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
	bool CreateFootprint(const FVector& Location, float FootSize);

	// Physics Integration
	UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
	void ApplyTerrainPhysics(class UPrimitiveComponent* Component, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
	float GetMovementSpeedMultiplier(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
	float GetStaminaDrainMultiplier(const FVector& Location);

	// Terrain Mapping
	UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
	void MapTerrainTypes();

	UFUNCTION(BlueprintCallable, Category = "Terrain Interaction")
	void UpdateTerrainCache(const FVector& CenterLocation, float Radius);

protected:
	// Terrain Type Mapping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	TMap<ECore_TerrainType, FCore_TerrainProperties> TerrainTypeMap;

	// Detection Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float TerrainDetectionRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float TerrainUpdateFrequency = 2.0f;

	// Footprint Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint Settings", meta = (ClampMin = "5.0", ClampMax = "50.0"))
	float FootprintRadius = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint Settings", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float FootprintDepth = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint Settings", meta = (ClampMin = "10.0", ClampMax = "300.0"))
	float FootprintDuration = 60.0f;

	// Digging Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Settings", meta = (ClampMin = "10.0", ClampMax = "200.0"))
	float MaxDigRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Settings", meta = (ClampMin = "5.0", ClampMax = "100.0"))
	float MaxDigDepth = 25.0f;

	// Cache System
	UPROPERTY()
	TMap<FVector, ECore_TerrainType> TerrainTypeCache;

	UPROPERTY()
	float LastTerrainUpdate = 0.0f;

	UPROPERTY()
	ALandscape* CachedLandscape = nullptr;

private:
	// Internal Methods
	ECore_TerrainType SampleTerrainFromLandscape(const FVector& Location);
	void InitializeTerrainTypeMap();
	bool IsLocationValid(const FVector& Location);
	FVector GetLandscapeLocation(const FVector& WorldLocation);
};