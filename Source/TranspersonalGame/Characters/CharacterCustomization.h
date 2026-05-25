#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "CharacterCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Athletic,
    Muscular,
    Lean,
    Stocky
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Fair,
    Medium,
    Tan,
    Dark,
    Weathered
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ClothingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMesh* TorsoMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMesh* LegsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    UMaterialInterface* ClothingMaterial;

    FChar_ClothingSet()
    {
        ClothingName = TEXT("Default");
        TorsoMesh = nullptr;
        LegsMesh = nullptr;
        ClothingMaterial = nullptr;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterCustomization : public UDataAsset
{
    GENERATED_BODY()

public:
    UCharacterCustomization();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    USkeletalMesh* BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FChar_ClothingSet> AvailableClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    USkeletalMesh* WeaponMesh;

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyCustomizationToCharacter(class ATranspersonalCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Character")
    FChar_ClothingSet GetClothingByName(const FString& ClothingName) const;
};

#include "CharacterCustomization.generated.h"