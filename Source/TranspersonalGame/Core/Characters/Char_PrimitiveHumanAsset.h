#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Char_PrimitiveHumanAsset.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light Tan"),
    Medium      UMETA(DisplayName = "Medium Brown"),
    Dark        UMETA(DisplayName = "Dark Bronze"),
    Weathered   UMETA(DisplayName = "Sun-Weathered")
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean Hunter"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Stocky      UMETA(DisplayName = "Stocky Gatherer"),
    Tall        UMETA(DisplayName = "Tall Scout")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Leather"),
    Decorated   UMETA(DisplayName = "Bone Decorated"),
    Practical   UMETA(DisplayName = "Practical Wraps"),
    Ceremonial  UMETA(DisplayName = "Ceremonial Paint")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PrimitiveHumanSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    EChar_BodyType BodyType = EChar_BodyType::Muscular;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::Practical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float ScarIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTattoos = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBoneJewelry = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bCarriesStoneAxe = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bCarriesSpear = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bCarriesBow = false;
};

/**
 * Data asset defining primitive human character appearance and equipment
 * Used by TranspersonalCharacter for visual customization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_PrimitiveHumanAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UChar_PrimitiveHumanAsset();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Mesh")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<EChar_SkinTone, TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<EChar_ClothingStyle, TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Meshes")
    TSoftObjectPtr<USkeletalMesh> StoneAxeMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Meshes")
    TSoftObjectPtr<USkeletalMesh> SpearMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment Meshes")
    TSoftObjectPtr<USkeletalMesh> BowMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessories")
    TArray<TSoftObjectPtr<USkeletalMesh>> BoneJewelryMeshes;

    // Apply character specification to skeletal mesh component
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyCharacterSpec(USkeletalMeshComponent* MeshComponent, const FChar_PrimitiveHumanSpec& Spec);

    // Generate random primitive human specification
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    static FChar_PrimitiveHumanSpec GenerateRandomSpec();

    // Get material for skin tone
    UFUNCTION(BlueprintCallable, Category = "Materials")
    UMaterialInterface* GetSkinMaterial(EChar_SkinTone SkinTone);

    // Get material for clothing style
    UFUNCTION(BlueprintCallable, Category = "Materials")
    UMaterialInterface* GetClothingMaterial(EChar_ClothingStyle ClothingStyle);

protected:
    // Default character specifications for different archetypes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FChar_PrimitiveHumanSpec HunterPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FChar_PrimitiveHumanSpec WarriorPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FChar_PrimitiveHumanSpec GathererPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FChar_PrimitiveHumanSpec ShamanPreset;
};