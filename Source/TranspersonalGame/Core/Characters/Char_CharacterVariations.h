#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Char_CharacterVariations.generated.h"

// Character archetype definitions for tribal survivors
UENUM(BlueprintType)
enum class EChar_TribalArchetype : uint8
{
    YoungHunter     UMETA(DisplayName = "Young Hunter"),
    ElderShaman     UMETA(DisplayName = "Elder Shaman"),
    FemaleGatherer  UMETA(DisplayName = "Female Gatherer"),
    ChildScout      UMETA(DisplayName = "Child Scout"),
    TribalWarrior   UMETA(DisplayName = "Tribal Warrior"),
    CraftsMaster    UMETA(DisplayName = "Crafts Master")
};

// Clothing material types based on available dinosaur hides
UENUM(BlueprintType)
enum class EChar_ClothingMaterial : uint8
{
    RaptorHide      UMETA(DisplayName = "Raptor Hide"),
    TricerHide      UMETA(DisplayName = "Triceratops Hide"),
    BrachiHide      UMETA(DisplayName = "Brachiosaurus Hide"),
    PteranodonWing  UMETA(DisplayName = "Pteranodon Wing"),
    PlantFiber      UMETA(DisplayName = "Plant Fiber"),
    BoneArmor       UMETA(DisplayName = "Bone Armor")
};

// Weapon and tool categories for character equipment
UENUM(BlueprintType)
enum class EChar_WeaponType : uint8
{
    StoneSpear      UMETA(DisplayName = "Stone Spear"),
    BoneClub        UMETA(DisplayName = "Bone Club"),
    FlintKnife      UMETA(DisplayName = "Flint Knife"),
    WoodenBow       UMETA(DisplayName = "Wooden Bow"),
    SlingSling      UMETA(DisplayName = "Sling"),
    FireTorch       UMETA(DisplayName = "Fire Torch")
};

// Character variation data structure
USTRUCT(BlueprintType)
struct FChar_CharacterVariation
{
    GENERATED_BODY()

    // Basic archetype information
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    EChar_TribalArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    FText Description;

    // Physical appearance
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    float HeightScale;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    float BodyMassScale;

    // Clothing and equipment
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    EChar_ClothingMaterial PrimaryClothing;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    EChar_WeaponType PrimaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    TArray<TSoftObjectPtr<UStaticMesh>> AccessoryMeshes;

    // Survival stats modifiers
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float HealthModifier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float StaminaModifier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float HungerResistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float ColdResistance;

    FChar_CharacterVariation()
    {
        Archetype = EChar_TribalArchetype::YoungHunter;
        CharacterName = TEXT("Unnamed Survivor");
        Description = FText::FromString(TEXT("A tribal survivor"));
        HeightScale = 1.0f;
        BodyMassScale = 1.0f;
        PrimaryClothing = EChar_ClothingMaterial::RaptorHide;
        PrimaryWeapon = EChar_WeaponType::StoneSpear;
        HealthModifier = 1.0f;
        StaminaModifier = 1.0f;
        HungerResistance = 1.0f;
        ColdResistance = 1.0f;
    }
};

/**
 * Data asset containing all character variation definitions
 * Used by character creation system to generate diverse NPCs and player options
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UChar_CharacterVariations : public UDataAsset
{
    GENERATED_BODY()

public:
    UChar_CharacterVariations();

    // Array of all available character variations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Variations")
    TArray<FChar_CharacterVariation> AvailableVariations;

    // Get a random character variation
    UFUNCTION(BlueprintCallable, Category = "Character Variations")
    FChar_CharacterVariation GetRandomVariation() const;

    // Get character variation by archetype
    UFUNCTION(BlueprintCallable, Category = "Character Variations")
    TArray<FChar_CharacterVariation> GetVariationsByArchetype(EChar_TribalArchetype Archetype) const;

    // Get character variation by name
    UFUNCTION(BlueprintCallable, Category = "Character Variations")
    bool GetVariationByName(const FString& Name, FChar_CharacterVariation& OutVariation) const;

protected:
    // Initialize default character variations
    void InitializeDefaultVariations();
};