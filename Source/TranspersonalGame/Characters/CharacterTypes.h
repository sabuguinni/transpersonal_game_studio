#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "CharacterTypes.generated.h"

/**
 * Prehistoric character archetypes for tribal NPCs and player customization
 */
UENUM(BlueprintType)
enum class EChar_TribalArchetype : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Warrior     UMETA(DisplayName = "Warrior"), 
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Scout       UMETA(DisplayName = "Scout"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter")
};

/**
 * Character physical attributes for procedural generation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PhysicalTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Height = 1.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Weight = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float MuscleDefinition = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float ScarIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    FChar_PhysicalTraits()
    {
        Height = 1.75f;
        Weight = 70.0f;
        MuscleDefinition = 0.5f;
        ScarIntensity = 0.3f;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
    }
};

/**
 * Clothing and equipment configuration for tribal characters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalOutfit
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outfit")
    TSoftObjectPtr<UStaticMesh> HeadgearMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outfit")
    TSoftObjectPtr<UStaticMesh> TorsoClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outfit")
    TSoftObjectPtr<UStaticMesh> LegClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outfit")
    TSoftObjectPtr<UStaticMesh> FootwearMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outfit")
    TSoftObjectPtr<UStaticMesh> WeaponMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outfit")
    TSoftObjectPtr<UStaticMesh> AccessoryMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outfit")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    FChar_TribalOutfit()
    {
        HeadgearMesh = nullptr;
        TorsoClothingMesh = nullptr;
        LegClothingMesh = nullptr;
        FootwearMesh = nullptr;
        WeaponMesh = nullptr;
        AccessoryMesh = nullptr;
        ClothingMaterial = nullptr;
    }
};

/**
 * Complete character configuration for spawning tribal NPCs
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalCharacterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Config")
    EChar_TribalArchetype Archetype = EChar_TribalArchetype::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Config")
    FChar_PhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Config")
    FChar_TribalOutfit Outfit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Config")
    FString CharacterName = TEXT("Tribal Member");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Config")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Config")
    bool bIsMale = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Config")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Config")
    float SurvivalSkill = 0.7f;

    FChar_TribalCharacterConfig()
    {
        Archetype = EChar_TribalArchetype::Hunter;
        CharacterName = TEXT("Tribal Member");
        Age = 25;
        bIsMale = true;
        AggressionLevel = 0.5f;
        SurvivalSkill = 0.7f;
    }
};

/**
 * Character appearance variations for diversity
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FLinearColor> SkinToneVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FLinearColor> HairColorVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<float> HeightVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<float> BuildVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<TSoftObjectPtr<UTexture2D>> FacialTextureVariations;

    FChar_AppearanceVariation()
    {
        // Initialize with default variations
        SkinToneVariations.Add(FLinearColor(0.8f, 0.6f, 0.4f, 1.0f));
        SkinToneVariations.Add(FLinearColor(0.6f, 0.4f, 0.3f, 1.0f));
        SkinToneVariations.Add(FLinearColor(0.9f, 0.7f, 0.5f, 1.0f));
        
        HairColorVariations.Add(FLinearColor(0.2f, 0.1f, 0.05f, 1.0f));
        HairColorVariations.Add(FLinearColor(0.4f, 0.3f, 0.2f, 1.0f));
        HairColorVariations.Add(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));
        
        HeightVariations.Add(1.6f);
        HeightVariations.Add(1.75f);
        HeightVariations.Add(1.9f);
        
        BuildVariations.Add(0.8f);
        BuildVariations.Add(1.0f);
        BuildVariations.Add(1.2f);
    }
};