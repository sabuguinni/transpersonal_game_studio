#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalWarriorAsset.generated.h"

/**
 * Asset descriptor for Cretaceous tribal warrior character
 * Contains mesh references and material specifications
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalWarriorAsset : public UObject
{
    GENERATED_BODY()

public:
    UChar_TribalWarriorAsset();

    /** Primary skeletal mesh for the warrior character */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<USkeletalMesh> WarriorMesh;

    /** Animal hide clothing material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> HideClothingMaterial;

    /** Bone jewelry material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BoneJewelryMaterial;

    /** Stone spear weapon mesh */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
    TSoftObjectPtr<UStaticMesh> StoneSpearMesh;

    /** Skin tone variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TArray<FLinearColor> SkinToneVariations;

    /** Hair color variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TArray<FLinearColor> HairColorVariations;

    /** Initialize default asset references */
    UFUNCTION(BlueprintCallable, Category = "Asset Management")
    void InitializeDefaultReferences();

    /** Validate all asset references are loaded */
    UFUNCTION(BlueprintCallable, Category = "Asset Management")
    bool ValidateAssetReferences() const;

    /** Get random skin tone variation */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    FLinearColor GetRandomSkinTone() const;

    /** Get random hair color variation */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    FLinearColor GetRandomHairColor() const;

protected:
    /** Setup default color variations */
    void SetupDefaultColorVariations();
};