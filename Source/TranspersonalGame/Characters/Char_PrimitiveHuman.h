#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Char_PrimitiveHuman.generated.h"

/**
 * Primitive human character for Cretaceous period survival game
 * Features realistic tribal appearance with weathered skin, hide clothing, and stone tools
 * Designed to work with MetaHuman Creator base or custom primitive human mesh
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PrimitiveHuman : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PrimitiveHuman();

protected:
    virtual void BeginPlay() override;

    // Character appearance components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Mesh")
    class USkeletalMeshComponent* CharacterMesh;

    // Clothing and accessories
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothing")
    class UStaticMeshComponent* HideClothing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessories")
    class UStaticMeshComponent* BoneJewelry;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tools")
    class UStaticMeshComponent* StoneAxe;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tools")
    class UStaticMeshComponent* BoneSpear;

    // Material variations for diversity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<class UMaterialInterface*> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<class UMaterialInterface*> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<class UStaticMesh*> JewelryVariations;

    // Character customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization", meta = (ClampMin = "0", ClampMax = "3"))
    int32 SkinVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization", meta = (ClampMin = "0", ClampMax = "3"))
    int32 ClothingVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    bool bHasJewelry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    bool bHasWeapons;

    // Physical characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float BodyScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float MuscleDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float WeatheringLevel;

public:
    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinVariation(int32 NewVariation);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetClothingVariation(int32 NewVariation);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ToggleJewelry(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ToggleWeapons(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Info")
    FString GetCharacterDescription() const;

protected:
    // Internal setup functions
    void SetupCharacterMesh();
    void SetupClothingAndAccessories();
    void ApplyMaterialVariations();
    void ConfigurePhysicalTraits();
};