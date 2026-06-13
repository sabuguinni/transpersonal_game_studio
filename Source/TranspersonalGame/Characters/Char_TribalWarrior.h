#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Char_TribalWarrior.generated.h"

/**
 * Primitive Cretaceous tribal warrior character
 * Represents a prehistoric human survivor with tribal equipment
 * Features realistic anatomy, tribal clothing, and stone age tools
 * Designed for MetaHuman integration with custom materials
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AChar_TribalWarrior : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalWarrior();

protected:
    virtual void BeginPlay() override;

    // === VISUAL COMPONENTS ===
    
    /** Tribal leather clothing mesh component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal Equipment")
    UStaticMeshComponent* TribalClothingMesh;
    
    /** Bone jewelry and ornaments mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal Equipment")
    UStaticMeshComponent* BoneJewelryMesh;
    
    /** Stone spear weapon mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal Weapons")
    UStaticMeshComponent* StoneSpearMesh;
    
    /** Stone axe tool mesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal Weapons")
    UStaticMeshComponent* StoneAxeMesh;
    
    // === MATERIALS ===
    
    /** Weathered skin material for prehistoric look */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Materials")
    UMaterialInterface* WeatheredSkinMaterial;
    
    /** Tribal leather clothing material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Materials")
    UMaterialInterface* TribalLeatherMaterial;
    
    /** Bone jewelry material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Materials")
    UMaterialInterface* BoneMaterial;
    
    /** Stone tools material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Materials")
    UMaterialInterface* StoneMaterial;
    
    // === CHARACTER TRAITS ===
    
    /** Warrior experience level (affects appearance and equipment quality) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Traits", meta = (ClampMin = "1", ClampMax = "10"))
    int32 WarriorLevel;
    
    /** Tribal affiliation identifier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Traits")
    FString TribeAffiliation;
    
    /** Survival experience (affects scars and weathering) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Traits", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SurvivalExperience;
    
    /** Hair style variation (for different tribal looks) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_HairStyle HairStyle;
    
    /** Body build variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild;
    
    /** Skin tone variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone;

public:
    // === CUSTOMIZATION FUNCTIONS ===
    
    /** Apply tribal warrior appearance preset */
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyTribalWarriorPreset();
    
    /** Randomize warrior appearance within tribal theme */
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RandomizeTribalAppearance();
    
    /** Set warrior experience level and update appearance */
    UFUNCTION(BlueprintCallable, Category = "Character Progression")
    void SetWarriorLevel(int32 NewLevel);
    
    /** Apply weather and survival damage to appearance */
    UFUNCTION(BlueprintCallable, Category = "Character Weathering")
    void ApplySurvivalWeathering(float WeatheringAmount);
    
    /** Equip/unequip stone weapons */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void SetWeaponVisibility(bool bShowSpear, bool bShowAxe);

protected:
    // === INTERNAL FUNCTIONS ===
    
    /** Initialize tribal equipment meshes */
    void InitializeTribalEquipment();
    
    /** Apply materials based on character traits */
    void UpdateCharacterMaterials();
    
    /** Configure MetaHuman base mesh for tribal appearance */
    void ConfigureMetaHumanMesh();
    
    /** Setup attachment points for tribal equipment */
    void SetupEquipmentAttachments();
};

// TODO_ASSET_GENERATION_FAILED: Tribal warrior 3D model generation failed
// Required assets for manual creation:
// - Tribal leather clothing mesh (loincloth, chest wrap, arm guards)
// - Bone jewelry set (necklace, bracelet, earrings, nose ring)
// - Stone spear with wooden shaft and flint tip
// - Stone hand axe with leather grip wrap
// - Weathered skin material with scars and tan
// - Tribal leather material with wear patterns
// - Bone material with age yellowing
// - Stone material with chipped edges and blood stains