#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PrehistoricCharacterDefinitions.generated.h"

/**
 * EChar_BodyType — Physical build categories for prehistoric human characters.
 * Prefix: EChar_ (Character Artist Agent #09)
 */
UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean Hunter"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Stocky      UMETA(DisplayName = "Stocky Gatherer"),
    Wiry        UMETA(DisplayName = "Wiry Scout")
};

/**
 * EChar_SkinTone — Skin tone variants for visual diversity.
 */
UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    VeryLight   UMETA(DisplayName = "Very Light"),
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Tan         UMETA(DisplayName = "Tan"),
    Dark        UMETA(DisplayName = "Dark"),
    VeryDark    UMETA(DisplayName = "Very Dark")
};

/**
 * EChar_HairStyle — Hair styles for prehistoric humans.
 */
UENUM(BlueprintType)
enum class EChar_HairStyle : uint8
{
    ShortMatted     UMETA(DisplayName = "Short Matted"),
    LongLoose       UMETA(DisplayName = "Long Loose"),
    BraidedBone     UMETA(DisplayName = "Braided with Bone Ornaments"),
    Dreadlocked     UMETA(DisplayName = "Dreadlocked"),
    ShornRough      UMETA(DisplayName = "Shorn Rough")
};

/**
 * EChar_ClothingType — Clothing types based on available prehistoric materials.
 */
UENUM(BlueprintType)
enum class EChar_ClothingType : uint8
{
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    WovenGrass      UMETA(DisplayName = "Woven Grass"),
    BarkCloth       UMETA(DisplayName = "Bark Cloth"),
    FeatherAdorned  UMETA(DisplayName = "Feather Adorned"),
    Minimal         UMETA(DisplayName = "Minimal — Survival Only")
};

/**
 * EChar_ScarPattern — Battle/survival scar patterns visible on skin.
 */
UENUM(BlueprintType)
enum class EChar_ScarPattern : uint8
{
    None            UMETA(DisplayName = "No Scars"),
    FaceSlash       UMETA(DisplayName = "Face Slash"),
    ArmBites        UMETA(DisplayName = "Arm Bite Marks"),
    TorsoGashes     UMETA(DisplayName = "Torso Gashes"),
    HeavilyScarred  UMETA(DisplayName = "Heavily Scarred — Veteran")
};

/**
 * FChar_AppearanceData — Complete visual definition for a prehistoric human character.
 * Used by MetaHuman customization pipeline and character selection screen.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    FName CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Lean;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    EChar_HairStyle HairStyle = EChar_HairStyle::LongLoose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    EChar_ClothingType Clothing = EChar_ClothingType::AnimalHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    EChar_ScarPattern Scars = EChar_ScarPattern::None;

    /** Height scale multiplier (0.85 = short, 1.0 = average, 1.15 = tall) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance", meta = (ClampMin = "0.75", ClampMax = "1.25"))
    float HeightScale = 1.0f;

    /** Muscle definition blend (0.0 = lean, 1.0 = maximum muscle) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MuscleDefinition = 0.5f;

    /** Age appearance (0.0 = young adult ~18, 1.0 = elder ~60) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AgeBlend = 0.2f;

    /** Whether this character is female */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    bool bIsFemale = false;

    /** Primary skin material tint color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    FLinearColor SkinTintColor = FLinearColor(0.8f, 0.65f, 0.5f, 1.0f);

    /** Hair color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    FLinearColor HairColor = FLinearColor(0.05f, 0.03f, 0.02f, 1.0f);

    /** Body paint/marking color (tribal markings) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
    FLinearColor BodyPaintColor = FLinearColor(0.4f, 0.2f, 0.1f, 0.0f);

    /** Dirt/grime accumulation (0.0 = clean, 1.0 = heavily soiled) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtLevel = 0.3f;

    /** Blood stain intensity from combat (0.0 = none, 1.0 = heavy) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BloodStainLevel = 0.0f;
};

/**
 * FChar_NPCAppearanceRow — DataTable row for NPC appearance diversity.
 * Each row defines a unique NPC visual identity — no two NPCs should look identical.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_NPCAppearanceRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Appearance")
    FChar_AppearanceData AppearanceData;

    /** NPC role description (Hunter, Elder, Scout, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    FText RoleDescription;

    /** Backstory hint visible in character inspection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    FText BackstoryHint;
};
