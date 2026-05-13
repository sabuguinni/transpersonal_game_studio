#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CharacterTypes.generated.h"

/**
 * Character archetype definitions for prehistoric tribal survivors
 */
UENUM(BlueprintType)
enum class EChar_TribalArchetype : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Scout       UMETA(DisplayName = "Scout"),
    Gatherer    UMETA(DisplayName = "Gatherer")
};

/**
 * Physical build types for character variation
 */
UENUM(BlueprintType)
enum class EChar_PhysicalBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Tall        UMETA(DisplayName = "Tall"),
    Short       UMETA(DisplayName = "Short")
};

/**
 * Age categories for character generation
 */
UENUM(BlueprintType)
enum class EChar_AgeCategory : uint8
{
    Child       UMETA(DisplayName = "Child (8-15)"),
    YoungAdult  UMETA(DisplayName = "Young Adult (16-25)"),
    Adult       UMETA(DisplayName = "Adult (26-40)"),
    MiddleAged  UMETA(DisplayName = "Middle Aged (41-55)"),
    Elder       UMETA(DisplayName = "Elder (56+)")
};

/**
 * Clothing and equipment styles
 */
UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    BasicHides      UMETA(DisplayName = "Basic Animal Hides"),
    LeatherArmor    UMETA(DisplayName = "Leather Armor"),
    FurCloak        UMETA(DisplayName = "Fur Cloak"),
    BoneDecorated   UMETA(DisplayName = "Bone Decorated"),
    TribalRobes     UMETA(DisplayName = "Tribal Robes"),
    HunterGear      UMETA(DisplayName = "Hunter Gear")
};

/**
 * Character appearance data structure for procedural generation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData : public FTableRowBase
{
    GENERATED_BODY()

    FChar_AppearanceData()
    {
        Archetype = EChar_TribalArchetype::Hunter;
        PhysicalBuild = EChar_PhysicalBuild::Muscular;
        AgeCategory = EChar_AgeCategory::Adult;
        ClothingStyle = EChar_ClothingStyle::BasicHides;
        Height = 175.0f;
        Weight = 70.0f;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        bHasScars = false;
        bHasTattoos = false;
        bHasFacePaint = false;
    }

    /** Character archetype */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_TribalArchetype Archetype;

    /** Physical build type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_PhysicalBuild PhysicalBuild;

    /** Age category */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_AgeCategory AgeCategory;

    /** Clothing and equipment style */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_ClothingStyle ClothingStyle;

    /** Character height in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "120.0", ClampMax = "220.0"))
    float Height;

    /** Character weight in kg */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "30.0", ClampMax = "150.0"))
    float Weight;

    /** Skin tone color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    /** Hair color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    /** Has visible scars */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
    bool bHasScars;

    /** Has tribal tattoos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
    bool bHasTattoos;

    /** Has face paint */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
    bool bHasFacePaint;

    /** Character name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    /** Brief background description */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString BackgroundDescription;
};

/**
 * Equipment loadout for different character types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_EquipmentLoadout
{
    GENERATED_BODY()

    FChar_EquipmentLoadout()
    {
        PrimaryWeapon = TEXT("Stone Spear");
        SecondaryWeapon = TEXT("Stone Knife");
        bHasShield = false;
        bHasBag = true;
    }

    /** Primary weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString PrimaryWeapon;

    /** Secondary weapon or tool */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString SecondaryWeapon;

    /** Has shield or protective gear */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bHasShield;

    /** Has carrying bag or pouch */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bHasBag;

    /** List of tools carried */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FString> Tools;

    /** List of consumables carried */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FString> Consumables;
};