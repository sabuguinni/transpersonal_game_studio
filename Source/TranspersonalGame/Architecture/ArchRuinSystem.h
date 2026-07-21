#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "ArchRuinSystem.generated.h"

/**
 * Arch_RuinElementType — type of ruin structural element
 * Prefix: Arch_ (Architecture & Interior Agent #07)
 */
UENUM(BlueprintType)
enum class EArch_RuinElementType : uint8
{
    StandingPillar      UMETA(DisplayName = "Standing Pillar"),
    FallenColumn        UMETA(DisplayName = "Fallen Column"),
    BrokenWall          UMETA(DisplayName = "Broken Wall"),
    ArchGateway         UMETA(DisplayName = "Arch Gateway"),
    RubblePile          UMETA(DisplayName = "Rubble Pile"),
    AltarSlab           UMETA(DisplayName = "Altar Slab")
};

/**
 * FArch_RuinElementData — data for a single ruin element
 */
USTRUCT(BlueprintType)
struct FArch_RuinElementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    EArch_RuinElementType ElementType = EArch_RuinElementType::StandingPillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    FVector RelativeOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    float WeatheringLevel = 0.5f;
};

/**
 * AArchRuinCluster — a cluster of Cretaceous-era stone ruins placed in the world.
 * Represents ancient pre-human structures reclaimed by jungle vegetation.
 * Used by Architecture & Interior Agent #07 to populate biome zones.
 */
UCLASS(ClassGroup = "TranspersonalGame|Architecture", meta = (DisplayName = "Arch Ruin Cluster"))
class TRANSPERSONALGAME_API AArchRuinCluster : public AActor
{
    GENERATED_BODY()

public:
    AArchRuinCluster();

    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    /** Rebuild the ruin cluster from element data */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Ruins")
    void RebuildCluster();

    /** Randomize element positions within scatter radius */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Ruins")
    void RandomizeLayout();

    /** List of ruin elements in this cluster */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    TArray<FArch_RuinElementData> RuinElements;

    /** Radius within which elements are scattered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float ScatterRadius = 800.0f;

    /** Overall weathering level (0=fresh, 1=fully degraded) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ClusterWeathering = 0.7f;

    /** Whether vines/moss should be applied to meshes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    bool bApplyVegetationOverlay = true;

    /** Biome zone identifier for this cluster */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    FName BiomeZoneID = NAME_None;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture|Ruins", meta = (AllowPrivateAccess = "true"))
    TArray<UStaticMeshComponent*> RuinMeshComponents;

    void ClearMeshComponents();
    void SpawnElementMesh(const FArch_RuinElementData& ElementData, int32 Index);
};
