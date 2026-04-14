#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "ArchitectureTypes.generated.h"

/**
 * Types of prehistoric structures that can be built
 */
UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None            UMETA(DisplayName = "None"),
    TribalHut       UMETA(DisplayName = "Tribal Hut"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    PlatformHouse   UMETA(DisplayName = "Platform House"),
    StoragePit      UMETA(DisplayName = "Storage Pit"),
    CookingArea     UMETA(DisplayName = "Cooking Area"),
    CraftingStation UMETA(DisplayName = "Crafting Station"),
    Palisade        UMETA(DisplayName = "Palisade"),
    WatchTower      UMETA(DisplayName = "Watch Tower"),
    RitualCircle    UMETA(DisplayName = "Ritual Circle"),
    BurialMound     UMETA(DisplayName = "Burial Mound"),
    StoneMegaliths  UMETA(DisplayName = "Stone Megaliths"),
    Ruins           UMETA(DisplayName = "Ruins")
};

/**
 * Construction materials available in the stone age
 */
UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    None        UMETA(DisplayName = "None"),
    Wood        UMETA(DisplayName = "Wood"),
    Stone       UMETA(DisplayName = "Stone"),
    Clay        UMETA(DisplayName = "Clay"),
    AnimalHide  UMETA(DisplayName = "Animal Hide"),
    PlantFiber  UMETA(DisplayName = "Plant Fiber"),
    Bone        UMETA(DisplayName = "Bone"),
    Thatch      UMETA(DisplayName = "Thatch"),
    Mud         UMETA(DisplayName = "Mud")
};

/**
 * Structural integrity levels
 */
UENUM(BlueprintType)
enum class EArch_StructuralIntegrity : uint8
{
    Collapsed   UMETA(DisplayName = "Collapsed"),
    Damaged     UMETA(DisplayName = "Damaged"),
    Weathered   UMETA(DisplayName = "Weathered"),
    Stable      UMETA(DisplayName = "Stable"),
    Pristine    UMETA(DisplayName = "Pristine")
};

/**
 * Age categories for structures
 */
UENUM(BlueprintType)
enum class EArch_StructureAge : uint8
{
    New         UMETA(DisplayName = "New"),
    Recent      UMETA(DisplayName = "Recent"),
    Established UMETA(DisplayName = "Established"),
    Old         UMETA(DisplayName = "Old"),
    Ancient     UMETA(DisplayName = "Ancient"),
    Prehistoric UMETA(DisplayName = "Prehistoric")
};

/**
 * Interior layout types for dwellings
 */
UENUM(BlueprintType)
enum class EArch_InteriorLayout : uint8
{
    Empty           UMETA(DisplayName = "Empty"),
    SingleRoom      UMETA(DisplayName = "Single Room"),
    SleepingArea    UMETA(DisplayName = "Sleeping Area"),
    CookingArea     UMETA(DisplayName = "Cooking Area"),
    StorageArea     UMETA(DisplayName = "Storage Area"),
    WorkshopArea    UMETA(DisplayName = "Workshop Area"),
    RitualArea      UMETA(DisplayName = "Ritual Area"),
    MultiPurpose    UMETA(DisplayName = "Multi Purpose")
};

/**
 * Structure data for procedural generation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<EArch_MaterialType> PrimaryMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    EArch_StructuralIntegrity Integrity = EArch_StructuralIntegrity::Stable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    EArch_StructureAge Age = EArch_StructureAge::Established;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorLayout InteriorLayout = EArch_InteriorLayout::Empty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inhabitants")
    int32 MaxOccupants = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inhabitants")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Functionality")
    bool bHasFire = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Functionality")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Functionality")
    bool bIsDefensive = false;

    FArch_StructureData()
    {
        PrimaryMaterials.Add(EArch_MaterialType::Wood);
    }
};

/**
 * Interior furnishing data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorFurnishing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Furnishing")
    FString FurnishingName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Furnishing")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Furnishing")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Furnishing")
    EArch_MaterialType Material = EArch_MaterialType::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Furnishing")
    bool bIsEssential = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Furnishing")
    float WearLevel = 0.0f; // 0.0 = new, 1.0 = completely worn
};

/**
 * Settlement layout data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_SettlementLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    TArray<FArch_StructureData> Structures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    int32 Population = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    bool bHasDefenses = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settlement")
    bool bIsAbandoned = false;
};