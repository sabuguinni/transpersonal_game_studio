#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalWarriorAsset.generated.h"

// TODO_ASSET_GENERATION_FAILED: Primitive Cretaceous tribal warrior character
// Full body T-pose, scientifically accurate human proportions
// Wearing simple animal hide clothing, bone jewelry, stone tools
// Weathered skin texture, realistic anatomy for game rigging
// Target polycount: 25000, realistic art style

/**
 * Asset specification for primitive tribal warrior character
 * Contains all visual elements needed for Cretaceous period human survivor
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalWarriorAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UChar_TribalWarriorAsset();

    // Base character mesh (full body with clothing)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Mesh")
    TSoftObjectPtr<USkeletalMesh> WarriorMesh;

    // Skin material variations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    // Clothing material variations (hide, fur, leather)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    // Bone jewelry materials
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> JewelryMaterials;

    // Stone tool materials
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> ToolMaterials;

    // Character customization options
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
    bool bHasFacePaint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
    bool bHasScars;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
    bool bHasTattoos;

    // Age and weathering level (0.0 = young, 1.0 = elder)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringLevel;

    // Gender variant
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
    bool bIsFemale;

    // Apply this asset configuration to a skeletal mesh component
    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void ApplyToSkeletalMesh(USkeletalMeshComponent* MeshComponent);

    // Get random material variant for specified category
    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    UMaterialInterface* GetRandomSkinMaterial() const;

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    UMaterialInterface* GetRandomClothingMaterial() const;

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    UMaterialInterface* GetRandomJewelryMaterial() const;

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    UMaterialInterface* GetRandomToolMaterial() const;
};