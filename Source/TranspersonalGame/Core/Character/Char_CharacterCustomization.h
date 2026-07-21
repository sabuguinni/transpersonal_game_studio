#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "UObject/NoExportTypes.h"
#include "Char_CharacterCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_CharacterGender : uint8
{
    Male        UMETA(DisplayName = "Male"),
    Female      UMETA(DisplayName = "Female")
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Heavy       UMETA(DisplayName = "Heavy")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Tanned      UMETA(DisplayName = "Tanned")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ClothingItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString SocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bIsEquipped;

    FChar_ClothingItem()
    {
        ItemName = TEXT("");
        SocketName = TEXT("");
        bIsEquipped = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_CharacterGender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FChar_ClothingItem> ClothingItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    FChar_CharacterAppearance()
    {
        Gender = EChar_CharacterGender::Male;
        BodyType = EChar_BodyType::Athletic;
        SkinTone = EChar_SkinTone::Medium;
        HairColor = FLinearColor::Black;
        EyeColor = FLinearColor::Brown;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_CharacterCustomization : public UObject
{
    GENERATED_BODY()

public:
    UChar_CharacterCustomization();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    FChar_CharacterAppearance CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    TArray<TSoftObjectPtr<USkeletalMesh>> MaleBaseMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    TArray<TSoftObjectPtr<USkeletalMesh>> FemaleBaseMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    TArray<FChar_ClothingItem> AvailableClothing;

    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetCharacterGender(EChar_CharacterGender NewGender);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHairColor(FLinearColor NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetEyeColor(FLinearColor NewEyeColor);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void EquipClothingItem(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void UnequipClothingItem(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyAppearanceToMesh(USkeletalMeshComponent* TargetMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    FChar_CharacterAppearance GetCurrentAppearance() const;

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void LoadAppearancePreset(const FChar_CharacterAppearance& PresetAppearance);

private:
    void UpdateBaseMesh();
    void UpdateSkinMaterial();
    void UpdateClothing();
};