#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalWarriorAsset.generated.h"

// TODO_ASSET_GENERATION_FAILED: Meshy and DALL-E both failed - manual asset creation required
// Asset Specifications:
// - Primitive Cretaceous tribal warrior character
// - Full body T-pose for rigging
// - Animal hide clothing (leather wraps, fur shoulder pads)
// - Bone jewelry (necklaces, arm bands, ear ornaments)
// - Stone spear weapon
// - Realistic human anatomy with weathered skin
// - Earth tone color palette (browns, tans, ochre)
// - Target polycount: 25,000 triangles
// - PBR textures: Diffuse, Normal, Roughness, AO

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalWarriorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> SpearMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UMaterialInterface> BoneJewelryMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimBlueprint> AnimationBlueprint;

    FChar_TribalWarriorConfig()
    {
        // Default placeholder paths - to be replaced when assets are created
        BodyMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath("/Engine/Characters/Mannequins/Meshes/SKM_Quinn"));
        SkinMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BasicShapes/BasicShapeMaterial"));
        ClothingMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BasicShapes/BasicShapeMaterial"));
        BoneJewelryMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Engine/BasicShapes/BasicShapeMaterial"));
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalWarriorAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UChar_TribalWarriorAsset();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Configuration")
    FChar_TribalWarriorConfig WarriorConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float BaseHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float BaseStamina = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SpearDamage = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    // Apply this configuration to a skeletal mesh component
    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void ApplyToSkeletalMesh(USkeletalMeshComponent* MeshComponent);

    // Get the configured skeletal mesh
    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    USkeletalMesh* GetBodyMesh();

    // Get the spear static mesh for attachment
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    UStaticMesh* GetSpearMesh();
};