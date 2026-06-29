#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchRuinInteriorManager.generated.h"

/**
 * Arch_RuinInteriorConfig — Configuration for a single prehistoric ruin interior cell
 * Defines the structural layout, props, and environmental storytelling elements.
 */
USTRUCT(BlueprintType)
struct FArch_RuinInteriorConfig
{
	GENERATED_BODY()

	/** Label for this ruin cell */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
	FString CellLabel;

	/** World location of the ruin cell center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
	FVector CellLocation;

	/** Radius of the ruin cell in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
	float CellRadius = 400.0f;

	/** Number of standing columns in this cell */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
	int32 NumColumns = 4;

	/** Whether this cell has a fire pit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
	bool bHasFirePit = true;

	/** Whether this cell has a stone altar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
	bool bHasAltar = false;

	/** Degradation level 0.0 (pristine) to 1.0 (fully ruined) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DegradationLevel = 0.75f;
};

/**
 * EArch_RuinType — Type of prehistoric ruin structure
 */
UENUM(BlueprintType)
enum class EArch_RuinType : uint8
{
	StoneShelter     UMETA(DisplayName = "Stone Shelter"),
	RuinedWall       UMETA(DisplayName = "Ruined Wall"),
	StoneCircle      UMETA(DisplayName = "Stone Circle"),
	CaveEntrance     UMETA(DisplayName = "Cave Entrance"),
	AncientAltar     UMETA(DisplayName = "Ancient Altar"),
	RockyOutcrop     UMETA(DisplayName = "Rocky Outcrop")
};

/**
 * AArchRuinInteriorManager
 * 
 * Architecture & Interior Agent (#7) — Manages placement and configuration of
 * prehistoric stone ruin interiors in the Cretaceous survival world.
 * 
 * Each ruin is a document of the creatures/humans that inhabited it.
 * Interior props tell the story of survival: fire pits, stone tools, bone piles.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Arch Ruin Interior Manager"))
class TRANSPERSONALGAME_API AArchRuinInteriorManager : public AActor
{
	GENERATED_BODY()

public:
	AArchRuinInteriorManager();

	/** Type of ruin this manager represents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
	EArch_RuinType RuinType = EArch_RuinType::StoneShelter;

	/** Configuration for the ruin interior layout */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruin")
	FArch_RuinInteriorConfig InteriorConfig;

	/** Array of spawned structural actors (walls, columns, floor) */
	UPROPERTY(BlueprintReadOnly, Category = "Architecture|Ruin")
	TArray<AActor*> SpawnedStructuralActors;

	/** Array of spawned prop actors (fire pit, altar, rubble) */
	UPROPERTY(BlueprintReadOnly, Category = "Architecture|Ruin")
	TArray<AActor*> SpawnedPropActors;

	/** Whether the ruin has been built in the world */
	UPROPERTY(BlueprintReadOnly, Category = "Architecture|Ruin")
	bool bIsBuilt = false;

	/** Build the ruin interior — spawns all structural and prop actors */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Ruin")
	void BuildRuinInterior();

	/** Clear all spawned actors for this ruin */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Ruin")
	void ClearRuinInterior();

	/** Get the degradation-adjusted wall height in cm */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture|Ruin")
	float GetAdjustedWallHeight() const;

	/** Get the number of intact columns based on degradation */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture|Ruin")
	int32 GetIntactColumnCount() const;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	/** Spawn a single structural wall segment */
	AActor* SpawnWallSegment(const FVector& Location, const FVector& Scale, const FString& Label);

	/** Spawn a column at the given location */
	AActor* SpawnColumn(const FVector& Location, float HeightScale, const FString& Label);

	/** Spawn rubble debris around the ruin */
	void SpawnRubbleDebris(int32 Count);

	/** Spawn fire pit prop */
	AActor* SpawnFirePit(const FVector& Location);

	/** Spawn stone altar prop */
	AActor* SpawnStoneAltar(const FVector& Location);
};
