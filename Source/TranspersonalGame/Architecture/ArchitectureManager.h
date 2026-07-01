#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "ArchitectureManager.generated.h"

/**
 * Enum for Cretaceous structure types.
 * Prefix: Arch_ to avoid conflicts with other agents.
 */
UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    RuinWall        UMETA(DisplayName = "Ruin Wall"),
    RuinArch        UMETA(DisplayName = "Ruin Arch"),
    RuinPillar      UMETA(DisplayName = "Ruin Pillar"),
    RuinFoundation  UMETA(DisplayName = "Ruin Foundation"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    Outcrop         UMETA(DisplayName = "Rocky Outcrop")
};

/**
 * Struct for a single ruin block placement.
 */
USTRUCT(BlueprintType)
struct FArch_RuinBlock
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector RelativeOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_StructureType BlockType = EArch_StructureType::RuinWall;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bIsFallen = false;
};

/**
 * AArchitectureManager — manages placement and procedural generation
 * of Cretaceous-era architectural ruins and stone structures.
 * Agent #7 — Architecture & Interior Agent
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitectureManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Spawn a ruin cluster at the given world location */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnRuinCluster(FVector WorldLocation, EArch_StructureType StructureType, int32 BlockCount);

    /** Clear all spawned ruin actors managed by this manager */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllRuins();

    /** Get the total number of spawned ruin actors */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture")
    int32 GetSpawnedRuinCount() const;

    /** Generate a procedural ruin wall from a start to end point */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateRuinWall(FVector StartPoint, FVector EndPoint, float WallHeight, float DecayFactor);

    /** Place a single ruin block at world location */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* PlaceRuinBlock(FVector Location, FVector Scale, FRotator Rotation, EArch_StructureType BlockType);

protected:
    /** Default mesh for wall blocks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Meshes")
    UStaticMesh* WallBlockMesh;

    /** Default mesh for pillars */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Meshes")
    UStaticMesh* PillarMesh;

    /** Default mesh for fallen blocks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Meshes")
    UStaticMesh* FallenBlockMesh;

    /** All ruin actors spawned by this manager */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture|State",
        meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> SpawnedRuinActors;

    /** Block spacing for wall generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Config")
    float BlockSpacing = 100.0f;

    /** Maximum height for generated walls */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Config")
    float MaxWallHeight = 400.0f;

    /** Decay factor — higher = more blocks missing/fallen (0.0 = perfect, 1.0 = rubble) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Config",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DefaultDecayFactor = 0.35f;

    /** Biome spawn location for this manager's ruin cluster */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Config")
    FVector BiomeSpawnLocation = FVector(50000.0f, 50000.0f, 100.0f);

private:
    /** Internal helper: compute decay probability for a block at height */
    bool ShouldBlockDecay(float HeightFraction, float DecayFactor) const;
};
