#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Char_TribalCustomization.generated.h"

USTRUCT(BlueprintType)
struct FChar_TribalMarkingPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    FString PatternName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    UTexture2D* PatternTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    FLinearColor DefaultColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Markings")
    float Intensity;

    FChar_TribalMarkingPattern()
    {
        PatternName = TEXT("None");
        PatternTexture = nullptr;
        DefaultColor = FLinearColor::White;
        Intensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ClothingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMesh* ChestPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMesh* LegPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMesh* FootPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    UMaterialInterface* ClothingMaterial;

    FChar_TribalClothingSet()
    {
        ClothingName = TEXT("Basic Hide");
        ChestPiece = nullptr;
        LegPiece = nullptr;
        FootPiece = nullptr;
        ClothingMaterial = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalWeaponSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
    FString WeaponSetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
    UStaticMesh* PrimaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
    UStaticMesh* SecondaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
    UStaticMesh* Shield;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
    UStaticMesh* RangedWeapon;

    FChar_TribalWeaponSet()
    {
        WeaponSetName = TEXT("Stone Age Basic");
        PrimaryWeapon = nullptr;
        SecondaryWeapon = nullptr;
        Shield = nullptr;
        RangedWeapon = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalCustomization : public UObject
{
    GENERATED_BODY()

public:
    UChar_TribalCustomization();

    // Customization Data Arrays
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Markings")
    TArray<FChar_TribalMarkingPattern> AvailableMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Clothing")
    TArray<FChar_TribalClothingSet> AvailableClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Weapons")
    TArray<FChar_TribalWeaponSet> AvailableWeapons;

    // Skin Tone Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Appearance")
    TArray<FLinearColor> SkinTonePresets;

    // Body Build Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization|Appearance")
    TArray<float> BodyBuildPresets;

    // Customization Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    FChar_TribalMarkingPattern GetMarkingPattern(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    FChar_TribalClothingSet GetClothingSet(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    FChar_TribalWeaponSet GetWeaponSet(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    FLinearColor GetSkinTonePreset(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    float GetBodyBuildPreset(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    void InitializeDefaultCustomizations();

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    FChar_TribalMarkingPattern CreateRandomMarkingPattern() const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    FLinearColor GenerateRandomTribalColor() const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    int32 GetMarkingPatternCount() const { return AvailableMarkings.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    int32 GetClothingSetCount() const { return AvailableClothing.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Tribal Customization")
    int32 GetWeaponSetCount() const { return AvailableWeapons.Num(); }

private:
    void SetupDefaultMarkings();
    void SetupDefaultClothing();
    void SetupDefaultWeapons();
    void SetupDefaultSkinTones();
    void SetupDefaultBodyBuilds();
};