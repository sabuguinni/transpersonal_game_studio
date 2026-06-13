#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalWarriorAsset.generated.h"

// TODO_ASSET_GENERATION_FAILED: Meshy and DALL-E APIs failed
// This placeholder defines the tribal warrior character asset specifications
// for manual implementation or future API retry

/**
 * Data asset defining visual specifications for Cretaceous tribal warrior characters
 * Contains mesh references, material parameters, and customization options
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalWarriorAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UChar_TribalWarriorAsset();

    // Base character mesh (to be replaced with generated tribal warrior)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Mesh")
    TSoftObjectPtr<UStaticMesh> WarriorBaseMesh;

    // Clothing and equipment meshes
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> AnimalHideClothing;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> BoneJewelry;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
    TSoftObjectPtr<UStaticMesh> StoneSpear;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
    TSoftObjectPtr<UStaticMesh> StoneAxe;

    // Material customization
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    // Visual customization parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinWeathering;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
    FLinearColor SkinTone;

    // Character stats affecting appearance
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MuscleMass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float BattleExperience;

    // Blueprint callable functions for runtime customization
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyRandomTribalVariation();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetScarPattern(int32 PatternIndex);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetTribalWeaponSet(int32 WeaponSetIndex);
};

/*
ASSET SPECIFICATIONS FOR MANUAL CREATION:

TRIBAL WARRIOR CHARACTER:
- Full body humanoid mesh, T-pose, ~25k polygons
- Muscular build adapted for prehistoric survival
- Weathered skin with ritual scars on face/arms
- Long braided hair with dinosaur feathers
- Animal hide clothing (simple wraps and loincloth)
- Bone jewelry (necklaces, arm bands, ear ornaments)
- Stone tools (spear, hand axe, knife)
- Earth tone color palette (browns, tans, ochre)
- Realistic human proportions
- Game-ready topology with proper UV mapping

MATERIAL REQUIREMENTS:
- PBR material setup with diffuse, normal, roughness
- Weathering and dirt overlay textures
- Scar detail normal maps
- Hair shader with proper alpha
- Clothing with wear patterns
- Bone/stone material variations

ANIMATION READY:
- Standard humanoid skeleton
- Proper bone weights
- T-pose for rigging
- Facial blend shapes for expressions
*/