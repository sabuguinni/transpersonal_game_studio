#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "ArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    RuinPillar      UMETA(DisplayName = "Ruin Pillar"),
    StoneWall       UMETA(DisplayName = "Stone Wall"),
    RockyOutcrop    UMETA(DisplayName = "Rocky Outcrop"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    AncientAltar    UMETA(DisplayName = "Ancient Altar"),
    BoulderCluster  UMETA(DisplayName = "Boulder Cluster")
};

UENUM(BlueprintType)
enum class EArch_DecayState : uint8
{
    Intact      UMETA(DisplayName = "Intact"),
    Weathered   UMETA(DisplayName = "Weathered"),
    Crumbling   UMETA(DisplayName = "Crumbling"),
    Ruined      UMETA(DisplayName = "Ruined"),
    Buried      UMETA(DisplayName = "Buried")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_StructureType StructureType = EArch_StructureType::RuinPillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_DecayState DecayState = EArch_DecayState::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float HeightMeters = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bHasPrehistoricFerns = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString MeshyAssetURL;
};

/**
 * AArch_ArchitectureManager
 * Manages placement and state of prehistoric architectural structures
 * (ruins, stone formations, cave entrances) in the Cretaceous world.
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

    /** Maximum number of structures to spawn in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Spawning")
    int32 MaxStructureCount = 50;

    /** Radius around player to activate structure LOD */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|LOD")
    float ActivationRadius = 10000.0f;

    /** Meshy GLB URL for the Cretaceous stone ruin pillar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Assets")
    FString CretaceousRuinPillarURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/meshes/1781924995225_Ancient_Cretaceous_stone_ruin_pillar__we.glb");

    /** Meshy task ID for tracking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Assets")
    FString MeshyTaskID = TEXT("019ee300-e685-7390-9303-299fa2fec0df");

    /** Register a new structure at the given location */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterStructure(FArch_StructureData StructureData);

    /** Get all structures within radius of a location */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetStructuresInRadius(FVector Center, float Radius) const;

    /** Spawn a ruin pillar at biome coordinates */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void SpawnRuinPillarAtBiomeCoords();

    /** Get structure count */
    UFUNCTION(BlueprintPure, Category = "Architecture")
    int32 GetStructureCount() const { return RegisteredStructures.Num(); }

private:
    /** Seed structures for the Cretaceous biome */
    void InitializeCretaceousStructures();

    /** Biome spawn coordinates (X=50000, Y=50000) */
    static constexpr float BiomeX = 50000.0f;
    static constexpr float BiomeY = 50000.0f;
    static constexpr float BiomeZ = 100.0f;
};
