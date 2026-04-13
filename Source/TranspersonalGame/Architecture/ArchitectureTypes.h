#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "ArchitectureTypes.generated.h"

/**
 * Enum for different types of prehistoric structures
 */
UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Dwelling        UMETA(DisplayName = "Dwelling"),
    Workshop        UMETA(DisplayName = "Workshop"),
    Storage         UMETA(DisplayName = "Storage"),
    Community       UMETA(DisplayName = "Community Hall"),
    Ceremonial      UMETA(DisplayName = "Ceremonial"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Cave            UMETA(DisplayName = "Cave Dwelling"),
    Megalith        UMETA(DisplayName = "Megalithic Structure")
};

/**
 * Enum for construction materials available in prehistoric era
 */
UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Wood            UMETA(DisplayName = "Wood"),
    Stone           UMETA(DisplayName = "Stone"),
    Clay            UMETA(DisplayName = "Clay/Mud"),
    Thatch          UMETA(DisplayName = "Thatch"),
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    Bone            UMETA(DisplayName = "Bone"),
    Natural         UMETA(DisplayName = "Natural Cave")
};

/**
 * Enum for structural condition/age
 */
UENUM(BlueprintType)
enum class EArch_StructureCondition : uint8
{
    New             UMETA(DisplayName = "New"),
    WellMaintained  UMETA(DisplayName = "Well Maintained"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Ruined          UMETA(DisplayName = "Ruined"),
    Ancient         UMETA(DisplayName = "Ancient Ruins")
};

/**
 * Structure data for building configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Dwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_MaterialType PrimaryMaterial = EArch_MaterialType::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_MaterialType SecondaryMaterial = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureCondition Condition = EArch_StructureCondition::WellMaintained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName = TEXT("Unnamed Structure");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructureAge = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsInhabited = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasInterior = true;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Dwelling;
        PrimaryMaterial = EArch_MaterialType::Wood;
        SecondaryMaterial = EArch_MaterialType::Stone;
        Condition = EArch_StructureCondition::WellMaintained;
        StructureName = TEXT("Unnamed Structure");
        StructureAge = 0.0f;
        MaxOccupants = 1;
        bIsInhabited = true;
        bHasInterior = true;
    }
};

/**
 * Interior furnishing data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FString> FurnishingItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFireplace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasWorkbench = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasBedding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float InteriorTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float LightLevel = 0.5f;

    FArch_InteriorData()
    {
        bHasFireplace = false;
        bHasWorkbench = false;
        bHasStorage = false;
        bHasBedding = false;
        InteriorTemperature = 20.0f;
        LightLevel = 0.5f;
    }
};