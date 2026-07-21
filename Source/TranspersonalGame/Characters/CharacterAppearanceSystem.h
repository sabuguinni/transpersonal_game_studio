#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "CharacterAppearanceSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Lean        UMETA(DisplayName = "Lean"),
    Stocky      UMETA(DisplayName = "Stocky")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Tan         UMETA(DisplayName = "Tan"),
    Dark        UMETA(DisplayName = "Dark"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_ClothingSet : uint8
{
    BasicHides      UMETA(DisplayName = "Basic Animal Hides"),
    HunterGear      UMETA(DisplayName = "Hunter Gear"),
    GathererRobes   UMETA(DisplayName = "Gatherer Robes"),
    ShamanAttire    UMETA(DisplayName = "Shaman Attire"),
    ScoutLeathers   UMETA(DisplayName = "Scout Leathers")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingSet ClothingSet = EChar_ClothingSet::BasicHides;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FVector BodyScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBoneAccessories = true;

    FChar_AppearanceData()
    {
        BodyType = EChar_BodyType::Athletic;
        SkinTone = EChar_SkinTone::Medium;
        ClothingSet = EChar_ClothingSet::BasicHides;
        BodyScale = FVector(1.0f, 1.0f, 1.0f);
        bHasTribalMarkings = false;
        bHasBoneAccessories = true;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterAppearanceSystem : public UDataAsset
{
    GENERATED_BODY()

public:
    UCharacterAppearanceSystem();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TMap<EChar_SkinTone, TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TMap<EChar_ClothingSet, TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meshes")
    TMap<EChar_ClothingSet, TSoftObjectPtr<UStaticMesh>> AccessoryMeshes;

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyAppearanceToCharacter(class ACharacter* Character, const FChar_AppearanceData& AppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    FChar_AppearanceData GenerateRandomAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    UMaterialInterface* GetSkinMaterial(EChar_SkinTone SkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    UMaterialInterface* GetClothingMaterial(EChar_ClothingSet ClothingSet);

private:
    void ApplyBodyScale(class ACharacter* Character, const FVector& Scale);
    void ApplySkinMaterial(class ACharacter* Character, EChar_SkinTone SkinTone);
    void ApplyClothingMaterial(class ACharacter* Character, EChar_ClothingSet ClothingSet);
    void ApplyTribalMarkings(class ACharacter* Character, bool bHasMarkings);
};

#include "CharacterAppearanceSystem.generated.h"