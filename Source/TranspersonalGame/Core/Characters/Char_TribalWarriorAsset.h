#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalWarriorAsset.generated.h"

// TODO_ASSET_GENERATION_FAILED: Primitive Cretaceous tribal warrior character
// Full body T-pose, scientifically accurate human proportions
// Wearing animal hide clothing with bone jewelry, carrying stone spear
// Realistic skin textures, neutral expression, game-ready low poly mesh ~25k polys

UENUM(BlueprintType)
enum class EChar_TribalRank : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Chief       UMETA(DisplayName = "Chief"),
    Elder       UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild = EChar_BodyBuild::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor WarPaintColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasWarPaint = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScarring = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleMass = 1.0f;

    FChar_TribalAppearance()
    {
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Athletic;
        WarPaintColor = FLinearColor::Red;
        bHasWarPaint = true;
        bHasScarring = false;
        MuscleMass = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalEquipment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> WeaponMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> ShieldMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> JewelryMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> HeadgearMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<USkeletalMesh> FootwearMesh;

    FChar_TribalEquipment()
    {
        WeaponMesh = nullptr;
        ShieldMesh = nullptr;
        ClothingMesh = nullptr;
        JewelryMesh = nullptr;
        HeadgearMesh = nullptr;
        FootwearMesh = nullptr;
    }
};

/**
 * Data asset defining a tribal warrior character configuration
 * Includes appearance settings, equipment loadouts, and material variants
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalWarriorAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UChar_TribalWarriorAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EChar_TribalRank Rank;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText Biography;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> MetalMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FChar_TribalEquipment Equipment;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float BaseHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float BaseStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float DefenseRating = 15.0f;

    UFUNCTION(BlueprintCallable, Category = "Character")
    FString GetCharacterDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool IsValidConfiguration() const;
};

#include "Char_TribalWarriorAsset.generated.h"