#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "CharacterTypes.generated.h"

/**
 * Enum defining different tribal character archetypes for the prehistoric world
 */
UENUM(BlueprintType)
enum class EChar_TribalArchetype : uint8
{
    Hunter          UMETA(DisplayName = "Hunter"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Elder           UMETA(DisplayName = "Elder/Shaman"),
    Scout           UMETA(DisplayName = "Scout"),
    Tracker         UMETA(DisplayName = "Tracker"),
    Child           UMETA(DisplayName = "Child"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Crafter         UMETA(DisplayName = "Crafter")
};

/**
 * Enum for character gender
 */
UENUM(BlueprintType)
enum class EChar_Gender : uint8
{
    Male            UMETA(DisplayName = "Male"),
    Female          UMETA(DisplayName = "Female")
};

/**
 * Enum for character age groups
 */
UENUM(BlueprintType)
enum class EChar_AgeGroup : uint8
{
    Child           UMETA(DisplayName = "Child (5-12)"),
    Young           UMETA(DisplayName = "Young Adult (13-25)"),
    Adult           UMETA(DisplayName = "Adult (26-45)"),
    Elder           UMETA(DisplayName = "Elder (46+)")
};

/**
 * Struct defining physical appearance traits for tribal characters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PhysicalTraits
{
    GENERATED_BODY()

    /** Base skin tone (0.0 = very dark, 1.0 = very light) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinTone = 0.6f;

    /** Hair color variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f); // Dark brown

    /** Eye color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f); // Brown

    /** Body build scale (0.8 = lean, 1.2 = muscular) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.7", ClampMax = "1.3"))
    float BodyBuild = 1.0f;

    /** Height scale relative to base character */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float HeightScale = 1.0f;

    /** Number of visible scars (survival experience) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0", ClampMax = "10"))
    int32 ScarCount = 2;

    /** Has tribal tattoos/markings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = true;

    FChar_PhysicalTraits()
    {
        SkinTone = 0.6f;
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        BodyBuild = 1.0f;
        HeightScale = 1.0f;
        ScarCount = 2;
        bHasTribalMarkings = true;
    }
};

/**
 * Struct defining clothing and equipment for tribal characters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalGear
{
    GENERATED_BODY()

    /** Primary clothing material (hide, fur, woven) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString ClothingMaterial = TEXT("Animal Hide");

    /** Primary weapon type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString PrimaryWeapon = TEXT("Stone Spear");

    /** Secondary tool */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString SecondaryTool = TEXT("Bone Knife");

    /** Jewelry/accessories description */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString Accessories = TEXT("Bone Necklace");

    /** Has ceremonial items (for elders/shamans) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bHasCeremonialItems = false;

    /** Clothing condition (0.0 = tattered, 1.0 = well-maintained) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ClothingCondition = 0.7f;

    FChar_TribalGear()
    {
        ClothingMaterial = TEXT("Animal Hide");
        PrimaryWeapon = TEXT("Stone Spear");
        SecondaryTool = TEXT("Bone Knife");
        Accessories = TEXT("Bone Necklace");
        bHasCeremonialItems = false;
        ClothingCondition = 0.7f;
    }
};

/**
 * Complete character definition for tribal NPCs and player customization
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalCharacterDefinition
{
    GENERATED_BODY()

    /** Character's name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName = TEXT("Unnamed Survivor");

    /** Character archetype */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EChar_TribalArchetype Archetype = EChar_TribalArchetype::Hunter;

    /** Character gender */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EChar_Gender Gender = EChar_Gender::Male;

    /** Character age group */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EChar_AgeGroup AgeGroup = EChar_AgeGroup::Adult;

    /** Physical appearance traits */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_PhysicalTraits PhysicalTraits;

    /** Clothing and equipment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FChar_TribalGear TribalGear;

    /** Character's survival experience level (affects scars, gear quality) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0", ClampMax = "100"))
    int32 ExperienceLevel = 25;

    /** Character's tribal status/rank */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FString TribalRank = TEXT("Hunter");

    /** Brief background description */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background", meta = (MultiLine = true))
    FString BackgroundStory = TEXT("A skilled survivor of the prehistoric world.");

    FChar_TribalCharacterDefinition()
    {
        CharacterName = TEXT("Unnamed Survivor");
        Archetype = EChar_TribalArchetype::Hunter;
        Gender = EChar_Gender::Male;
        AgeGroup = EChar_AgeGroup::Adult;
        ExperienceLevel = 25;
        TribalRank = TEXT("Hunter");
        BackgroundStory = TEXT("A skilled survivor of the prehistoric world.");
    }
};

/**
 * Preset character definitions for common tribal archetypes
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalPresets
{
    GENERATED_BODY()

    /** Get a preset character definition based on archetype and gender */
    static FChar_TribalCharacterDefinition GetPresetCharacter(EChar_TribalArchetype Archetype, EChar_Gender Gender, EChar_AgeGroup AgeGroup);

    /** Get random character definition with some variation */
    static FChar_TribalCharacterDefinition GetRandomCharacter();

    /** Apply random variations to an existing character definition */
    static void ApplyRandomVariations(FChar_TribalCharacterDefinition& CharacterDef, float VariationStrength = 0.3f);
};