#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "ArchitectureManager.generated.h"

/**
 * EArch_StructureType — Types of prehistoric architectural structures
 * Prefix: EArch_ (Architecture Agent #7)
 */
UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    RuinedPillar        UMETA(DisplayName = "Ruined Pillar"),
    StonePlatform       UMETA(DisplayName = "Stone Platform"),
    CaveEntrance        UMETA(DisplayName = "Cave Entrance"),
    RockyOutcrop        UMETA(DisplayName = "Rocky Outcrop"),
    AncientWallSegment  UMETA(DisplayName = "Ancient Wall Segment"),
    FallenMonolith      UMETA(DisplayName = "Fallen Monolith"),
    NaturalArch         UMETA(DisplayName = "Natural Arch"),
    ShelterOverhang     UMETA(DisplayName = "Shelter Overhang")
};

/**
 * EArch_WearState — Weathering/decay state of a structure
 */
UENUM(BlueprintType)
enum class EArch_WearState : uint8
{
    Pristine    UMETA(DisplayName = "Pristine"),
    Weathered   UMETA(DisplayName = "Weathered"),
    Crumbling   UMETA(DisplayName = "Crumbling"),
    Ruined      UMETA(DisplayName = "Ruined"),
    Collapsed   UMETA(DisplayName = "Collapsed")
};

/**
 * FArch_StructureData — Data for a single architectural structure instance
 */
USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_StructureType StructureType = EArch_StructureType::RuinedPillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_WearState WearState = EArch_WearState::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FRotator WorldRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bHasInterior = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bCanProvideShelte = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float ShelterRadius = 300.0f;
};

/**
 * AArch_ArchitectureManager — Manages prehistoric architectural structures in the world.
 * Spawns, tracks, and provides shelter/interaction queries for stone ruins,
 * rocky outcrops, cave entrances, and natural shelters.
 *
 * Architecture Agent #7 — Transpersonal Game Studio
 * Structures are documents of the world's history. Every ruin tells a story.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_ArchitectureManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** All registered structures in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Structures")
    TArray<FArch_StructureData> RegisteredStructures;

    /** Maximum number of structures to maintain in memory */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Performance")
    int32 MaxActiveStructures = 200;

    /** Radius around player to keep structures loaded */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Streaming")
    float StreamingRadius = 15000.0f;

    /** Register a new structure at runtime */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterStructure(const FArch_StructureData& StructureData);

    /** Find nearest shelter structure to a world location */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool FindNearestShelter(FVector FromLocation, float SearchRadius, FArch_StructureData& OutShelter);

    /** Get all structures of a specific type within radius */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetStructuresOfTypeInRadius(EArch_StructureType Type, FVector Center, float Radius);

    /** Spawn a structure actor in the world */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* SpawnStructureActor(const FArch_StructureData& StructureData);

    /** Count of currently registered structures */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture")
    int32 GetStructureCount() const;

    /** Clear all registered structures (use with caution) */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();

protected:
    /** Spawned actor references for cleanup */
    UPROPERTY()
    TArray<AActor*> SpawnedStructureActors;

private:
    /** Internal: find distance squared between two points */
    float DistSq(const FVector& A, const FVector& B) const;
};
