#pragma once

#include "CoreMinimal.h"
#include "HerdFormationData.generated.h"

/**
 * Herd behavior state for crowd simulation agents.
 * Used by Mass AI processor to drive herbivore herd dynamics.
 */
UENUM(BlueprintType)
enum class ECrowd_HerdBehavior : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Browsing    UMETA(DisplayName = "Browsing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
};

/**
 * Species classification for crowd simulation agents.
 */
UENUM(BlueprintType)
enum class ECrowd_HerbivoreSpecies : uint8
{
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Gallimimus      UMETA(DisplayName = "Gallimimus"),
};

/**
 * Data for a single herd formation slot.
 * Defines position offset from herd center, heading variation, and role.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdSlot
{
    GENERATED_BODY()

    /** Position offset from herd center in local space */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector LocalOffset = FVector::ZeroVector;

    /** Heading variation from herd's shared orientation (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float HeadingVariation = 0.0f;

    /** Role of this individual in the herd */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FName Role = NAME_None;
};

/**
 * Complete herd formation definition.
 * Describes the spatial layout and behavior of a herbivore herd.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdFormation
{
    GENERATED_BODY()

    /** Species this formation is for */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerbivoreSpecies Species = ECrowd_HerbivoreSpecies::Triceratops;

    /** World location of herd center */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector HerdCenter = FVector::ZeroVector;

    /** Shared heading direction (degrees, yaw only) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SharedHeading = 0.0f;

    /** Current behavior state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdBehavior CurrentBehavior = ECrowd_HerdBehavior::Grazing;

    /** Individual slots in this formation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    TArray<FCrowd_HerdSlot> Slots;

    /** Minimum spacing between individuals (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float MinSpacing = 30000.0f;

    /** Maximum spacing between individuals (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float MaxSpacing = 60000.0f;

    /** Alert radius — distance at which herd detects threats (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float AlertRadius = 500000.0f;
};
