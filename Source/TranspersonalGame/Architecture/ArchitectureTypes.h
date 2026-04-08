#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "ArchitectureTypes.generated.h"

/**
 * Enumeration of prehistoric structure types
 */
UENUM(BlueprintType)
enum class EPrehistoricStructureType : uint8
{
    None            UMETA(DisplayName = "None"),
    Cave            UMETA(DisplayName = "Cave Dwelling"),
    Hut             UMETA(DisplayName = "Primitive Hut"),
    Shelter         UMETA(DisplayName = "Rock Shelter"),
    Ceremonial      UMETA(DisplayName = "Ceremonial Site"),
    Storage         UMETA(DisplayName = "Storage Cache"),
    Workshop        UMETA(DisplayName = "Tool Workshop"),
    Burial          UMETA(DisplayName = "Burial Site"),
    Watchtower      UMETA(DisplayName = "Observation Post"),
    Bridge          UMETA(DisplayName = "Primitive Bridge"),
    Ruins           UMETA(DisplayName = "Ancient Ruins")
};

/**
 * Enumeration of construction materials used by prehistoric humans
 */
UENUM(BlueprintType)
enum class EPrehistoricMaterial : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Bone            UMETA(DisplayName = "Bone"),
    Hide            UMETA(DisplayName = "Animal Hide"),
    Clay            UMETA(DisplayName = "Clay"),
    Thatch          UMETA(DisplayName = "Thatch"),
    Mud             UMETA(DisplayName = "Mud Brick"),
    Organic         UMETA(DisplayName = "Organic Matter")
};

/**
 * Enumeration of preservation states for structures
 */
UENUM(BlueprintType)
enum class EStructureCondition : uint8
{
    Pristine        UMETA(DisplayName = "Recently Abandoned"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Partially Damaged"),
    Ruined          UMETA(DisplayName = "Heavily Ruined"),
    Collapsed       UMETA(DisplayName = "Collapsed"),
    Overgrown       UMETA(DisplayName = "Overgrown by Nature")
};

/**
 * Structure that defines an interior space and its story
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteriorNarrative
{
    GENERATED_BODY()

    // Who lived here
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString FormerInhabitant;

    // What happened to them
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString FateOfInhabitant;

    // Objects left behind that tell the story
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> StoryObjects;

    // Signs of struggle or peaceful departure
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bShowsSignsOfStruggle;

    // Time since abandonment (affects decay)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float YearsSinceAbandonment;

    FInteriorNarrative()
    {
        FormerInhabitant = TEXT("Unknown");
        FateOfInhabitant = TEXT("Disappeared");
        bShowsSignsOfStruggle = false;
        YearsSinceAbandonment = 10.0f;
    }
};

/**
 * Data asset that defines a prehistoric structure template
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPrehistoricStructureData : public UDataAsset
{
    GENERATED_BODY()

public:
    // Basic structure information
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EPrehistoricStructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString Description;

    // Physical properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    EPrehistoricMaterial PrimaryMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    EStructureCondition Condition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FVector Dimensions;

    // Mesh references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TSoftObjectPtr<UStaticMesh> ExteriorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<TSoftObjectPtr<UStaticMesh>> InteriorMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<TSoftObjectPtr<UStaticMesh>> PropMeshes;

    // Narrative content
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FInteriorNarrative InteriorStory;

    // Gameplay properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bCanProvideShel

;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bContainsResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bIsDangerous;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float SafetyRating; // 0.0 = death trap, 1.0 = completely safe

public:
    UPrehistoricStructureData()
    {
        StructureType = EPrehistoricStructureType::None;
        StructureName = TEXT("Unnamed Structure");
        Description = TEXT("A structure from the past");
        PrimaryMaterial = EPrehistoricMaterial::Stone;
        Condition = EStructureCondition::Weathered;
        Dimensions = FVector(500.0f, 500.0f, 300.0f);
        bCanProvideShel = true;
        bContainsResources = false;
        bIsDangerous = false;
        SafetyRating = 0.7f;
    }
};