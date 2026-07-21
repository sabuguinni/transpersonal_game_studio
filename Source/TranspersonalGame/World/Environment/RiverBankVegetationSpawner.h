#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RiverBankVegetationSpawner.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_VegetationType : uint8
{
	CycadPalm       UMETA(DisplayName = "Cycad Palm"),
	TreeFern        UMETA(DisplayName = "Tree Fern"),
	HorsetailReed   UMETA(DisplayName = "Horsetail Reed"),
	GiantFern       UMETA(DisplayName = "Giant Fern"),
	AquaticMoss     UMETA(DisplayName = "Aquatic Moss"),
	RiverGrass      UMETA(DisplayName = "River Grass")
};

USTRUCT(BlueprintType)
struct FEnvArt_VegetationEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	EEnvArt_VegetationType VegetationType = EEnvArt_VegetationType::CycadPalm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	FVector SpawnLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float HeightScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float RadiusScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	bool bIsWaterEdge = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARiverBankVegetationSpawner : public AActor
{
	GENERATED_BODY()

public:
	ARiverBankVegetationSpawner();

	virtual void BeginPlay() override;

	// River path configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Config")
	float RiverStartY = -5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Config")
	float RiverEndY = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Config")
	float RiverCenterX = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Config")
	float BankWidth = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Config")
	int32 VegetationDensity = 14;

	// Spawned vegetation registry
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "River|State")
	TArray<FEnvArt_VegetationEntry> SpawnedVegetation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "River|State")
	int32 TotalVegetationCount = 0;

	// Mesh references (set in Blueprint)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Meshes")
	TObjectPtr<UStaticMesh> CycadTrunkMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Meshes")
	TObjectPtr<UStaticMesh> CycadCrownMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Meshes")
	TObjectPtr<UStaticMesh> FernMesh;

	// Core methods
	UFUNCTION(BlueprintCallable, Category = "River|Vegetation")
	void SpawnRiverBankVegetation();

	UFUNCTION(BlueprintCallable, Category = "River|Vegetation")
	void ClearAllVegetation();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "River|Vegetation")
	void PreviewInEditor();

	UFUNCTION(BlueprintPure, Category = "River|Vegetation")
	bool IsLocationOnRiverBank(const FVector& Location) const;

	UFUNCTION(BlueprintPure, Category = "River|Vegetation")
	EEnvArt_VegetationType GetVegetationTypeForLocation(const FVector& Location) const;

	UFUNCTION(BlueprintCallable, Category = "River|Vegetation")
	int32 GetVegetationCount() const { return TotalVegetationCount; }

private:
	void SpawnSingleCycadPalm(const FVector& BaseLocation, float HeightMult);
	void SpawnSingleFern(const FVector& BaseLocation);
	TArray<AActor*> SpawnedActors;
};
