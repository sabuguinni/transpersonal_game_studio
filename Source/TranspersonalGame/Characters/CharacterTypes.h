#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CharacterTypes.generated.h"

/**
 * Character archetypes for prehistoric tribal survivors
 */
UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Elder       UMETA(DisplayName = "Elder"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Scout       UMETA(DisplayName = "Scout"),
    Child       UMETA(DisplayName = "Child")
};

/**
 * Physical build types for character variation
 */
UENUM(BlueprintType)
enum class EChar_BuildType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Tall        UMETA(DisplayName = "Tall"),
    Short       UMETA(DisplayName = "Short")
};

/**
 * Clothing styles based on available materials
 */
UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    BasicHide       UMETA(DisplayName = "Basic Hide"),
    WovenFiber      UMETA(DisplayName = "Woven Fiber"),
    CeremonialRobe  UMETA(DisplayName = "Ceremonial Robe"),
    HunterGear      UMETA(DisplayName = "Hunter Gear"),
    WarriorArmor    UMETA(DisplayName = "Warrior Armor"),
    ChildClothing   UMETA(DisplayName = "Child Clothing")
};

/**
 * Weapon specializations for different roles
 */
UENUM(BlueprintType)
enum class EChar_WeaponType : uint8
{
    Spear       UMETA(DisplayName = "Spear"),
    Bow         UMETA(DisplayName = "Bow"),
    Club        UMETA(DisplayName = "Club"),
    Knife       UMETA(DisplayName = "Stone Knife"),
    Sling       UMETA(DisplayName = "Sling"),
    Staff       UMETA(DisplayName = "Staff"),
    None        UMETA(DisplayName = "None")
};

/**
 * Data structure for character appearance customization
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData : public FTableRowBase
{
    GENERATED_BODY()

    /** Character's tribal role */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_TribalRole TribalRole = EChar_TribalRole::Hunter;

    /** Physical build type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_BuildType BuildType = EChar_BuildType::Athletic;

    /** Clothing style */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::BasicHide;

    /** Primary weapon type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_WeaponType PrimaryWeapon = EChar_WeaponType::Spear;

    /** Secondary weapon type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_WeaponType SecondaryWeapon = EChar_WeaponType::Knife;

    /** Skin tone variation (0.0 = lightest, 1.0 = darkest) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinTone = 0.5f;

    /** Hair color variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    /** Has tribal face paint */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFacePaint = false;

    /** Face paint color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor FacePaintColor = FLinearColor::Red;

    /** Has scars from encounters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = false;

    /** Age category (affects appearance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (ClampMin = "12", ClampMax = "70"))
    int32 Age = 25;

    /** Character name for identification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName = TEXT("Tribal Survivor");

    FChar_AppearanceData()
    {
        TribalRole = EChar_TribalRole::Hunter;
        BuildType = EChar_BuildType::Athletic;
        ClothingStyle = EChar_ClothingStyle::BasicHide;
        PrimaryWeapon = EChar_WeaponType::Spear;
        SecondaryWeapon = EChar_WeaponType::Knife;
        SkinTone = 0.5f;
        HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        bHasFacePaint = false;
        FacePaintColor = FLinearColor::Red;
        bHasScars = false;
        Age = 25;
        CharacterName = TEXT("Tribal Survivor");
    }
};

/**
 * Character stats based on role and build
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_StatsData
{
    GENERATED_BODY()

    /** Base health points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float BaseHealth = 100.0f;

    /** Base stamina points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float BaseStamina = 100.0f;

    /** Movement speed multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SpeedMultiplier = 1.0f;

    /** Strength for melee combat */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Strength = 50.0f;

    /** Agility for ranged combat and stealth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Agility = 50.0f;

    /** Survival skills for crafting and resource gathering */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Survival = 50.0f;

    FChar_StatsData()
    {
        BaseHealth = 100.0f;
        BaseStamina = 100.0f;
        SpeedMultiplier = 1.0f;
        Strength = 50.0f;
        Agility = 50.0f;
        Survival = 50.0f;
    }
};