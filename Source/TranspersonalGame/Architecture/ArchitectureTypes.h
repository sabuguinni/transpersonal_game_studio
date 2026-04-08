// Architecture & Interior Agent — Transpersonal Game Studio
// PROD_JURASSIC_001 — Architecture Types Definition
// Agent #07 — Architecture & Interior Agent

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "ArchitectureTypes.generated.h"

UENUM(BlueprintType)
enum class EArchitectureType : uint8
{
    // SURVIVAL STRUCTURES
    BasicShelter         UMETA(DisplayName = "Basic Shelter"),
    ReinforcedHut        UMETA(DisplayName = "Reinforced Hut"),
    WatchTower          UMETA(DisplayName = "Watch Tower"),
    StoragePit          UMETA(DisplayName = "Storage Pit"),
    
    // DEFENSIVE STRUCTURES
    Palisade            UMETA(DisplayName = "Palisade"),
    Barricade           UMETA(DisplayName = "Barricade"),
    TrapPit             UMETA(DisplayName = "Trap Pit"),
    SpikeTrap           UMETA(DisplayName = "Spike Trap"),
    
    // UTILITY STRUCTURES
    FirePit             UMETA(DisplayName = "Fire Pit"),
    WorkBench           UMETA(DisplayName = "Work Bench"),
    DryingRack          UMETA(DisplayName = "Drying Rack"),
    WaterCollector      UMETA(DisplayName = "Water Collector"),
    
    // RUINS & ABANDONED
    AbandonedCamp       UMETA(DisplayName = "Abandoned Camp"),
    CollapsedShelter    UMETA(DisplayName = "Collapsed Shelter"),
    BuriedStructure     UMETA(DisplayName = "Buried Structure"),
    AncientRuins        UMETA(DisplayName = "Ancient Ruins")
};

UENUM(BlueprintType)
enum class EInteriorType : uint8
{
    // LIVING SPACES
    SleepingArea        UMETA(DisplayName = "Sleeping Area"),
    CookingArea         UMETA(DisplayName = "Cooking Area"),
    StorageArea         UMETA(DisplayName = "Storage Area"),
    WorkArea            UMETA(DisplayName = "Work Area"),
    
    // DEFENSIVE INTERIORS
    HidingSpot          UMETA(DisplayName = "Hiding Spot"),
    LookoutPost         UMETA(DisplayName = "Lookout Post"),
    SafeRoom            UMETA(DisplayName = "Safe Room"),
    
    // ABANDONED INTERIORS
    EmptyDwelling       UMETA(DisplayName = "Empty Dwelling"),
    RansackedShelter    UMETA(DisplayName = "Ransacked Shelter"),
    OvergrownInterior   UMETA(DisplayName = "Overgrown Interior")
};

UENUM(BlueprintType)
enum class EConstructionMaterial : uint8
{
    // NATURAL MATERIALS
    Wood                UMETA(DisplayName = "Wood"),
    Stone               UMETA(DisplayName = "Stone"),
    Mud                 UMETA(DisplayName = "Mud"),
    Leaves              UMETA(DisplayName = "Leaves"),
    Bone                UMETA(DisplayName = "Bone"),
    Hide                UMETA(DisplayName = "Hide"),
    
    // COMPOSITE MATERIALS
    WoodAndMud          UMETA(DisplayName = "Wood and Mud"),
    StoneAndWood        UMETA(DisplayName = "Stone and Wood"),
    BoneAndHide         UMETA(DisplayName = "Bone and Hide")
};

USTRUCT(BlueprintType)
struct FArchitectureSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArchitectureType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EConstructionMaterial PrimaryMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EConstructionMaterial SecondaryMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Dimensions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatherResistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bIsDefensive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bIsAbandoned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<EInteriorType> InteriorSpaces;

    FArchitectureSpec()
    {
        Type = EArchitectureType::BasicShelter;
        PrimaryMaterial = EConstructionMaterial::Wood;
        SecondaryMaterial = EConstructionMaterial::Leaves;
        Dimensions = FVector(300.0f, 300.0f, 200.0f);
        StructuralIntegrity = 1.0f;
        WeatherResistance = 0.5f;
        bIsDefensive = false;
        bIsAbandoned = false;
    }
};

/**
 * Data Asset that defines architectural presets for the game
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UArchitectureDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Presets")
    TArray<FArchitectureSpec> ArchitecturePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Mapping")
    TMap<EConstructionMaterial, TSoftObjectPtr<UMaterialInterface>> MaterialMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Mapping")
    TMap<EArchitectureType, TSoftObjectPtr<UStaticMesh>> MeshMap;
};