#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/DataTable.h"
#include "Char_PrimitiveHumanCharacter.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    VeryLight UMETA(DisplayName = "Very Light"),
    Light UMETA(DisplayName = "Light"),
    Medium UMETA(DisplayName = "Medium"),
    Dark UMETA(DisplayName = "Dark"),
    VeryDark UMETA(DisplayName = "Very Dark")
};

UENUM(BlueprintType)
enum class EChar_WeatheringLevel : uint8
{
    Fresh UMETA(DisplayName = "Fresh"),
    Weathered UMETA(DisplayName = "Weathered"),
    Scarred UMETA(DisplayName = "Scarred"),
    BattleWorn UMETA(DisplayName = "Battle Worn")
};

UENUM(BlueprintType)
enum class EChar_TribalMarkings : uint8
{
    None UMETA(DisplayName = "None"),
    Hunter UMETA(DisplayName = "Hunter"),
    Gatherer UMETA(DisplayName = "Gatherer"),
    Shaman UMETA(DisplayName = "Shaman"),
    Warrior UMETA(DisplayName = "Warrior"),
    Elder UMETA(DisplayName = "Elder")
};

USTRUCT(BlueprintType)
struct FChar_CharacterCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_WeatheringLevel WeatheringLevel = EChar_WeatheringLevel::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalMarkings TribalMarkings = EChar_TribalMarkings::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleMass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float ScarVisibility = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor MarkingColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);

    FChar_CharacterCustomization()
    {
        SkinTone = EChar_SkinTone::Medium;
        WeatheringLevel = EChar_WeatheringLevel::Weathered;
        TribalMarkings = EChar_TribalMarkings::Hunter;
        MuscleMass = 1.0f;
        ScarVisibility = 0.5f;
        MarkingColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PrimitiveHumanCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PrimitiveHumanCharacter();

protected:
    virtual void BeginPlay() override;

    // Character customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    FChar_CharacterCustomization CharacterCustomization;

    // Clothing and equipment
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UStaticMeshComponent* AnimalHideClothing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UStaticMeshComponent* PrimitiveWeapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UStaticMeshComponent* TribalAccessories;

    // Materials for customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> SkinToneMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> WeatheringMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> TribalMarkingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ClothingMaterial;

    // Dynamic material instances
    UPROPERTY()
    UMaterialInstanceDynamic* DynamicSkinMaterial;

    UPROPERTY()
    UMaterialInstanceDynamic* DynamicClothingMaterial;

public:
    // Customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyCharacterCustomization(const FChar_CharacterCustomization& NewCustomization);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetWeatheringLevel(EChar_WeatheringLevel NewWeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetTribalMarkings(EChar_TribalMarkings NewMarkings);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetMuscleMass(float NewMuscleMass);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetScarVisibility(float NewScarVisibility);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetMarkingColor(FLinearColor NewColor);

    // Equipment functions
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipPrimitiveWeapon(UStaticMesh* WeaponMesh);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipTribalAccessories(UStaticMesh* AccessoryMesh);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UpdateClothing(UStaticMesh* ClothingMesh);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Character Customization")
    FChar_CharacterCustomization GetCharacterCustomization() const { return CharacterCustomization; }

    UFUNCTION(BlueprintPure, Category = "Character Customization")
    EChar_SkinTone GetSkinTone() const { return CharacterCustomization.SkinTone; }

    UFUNCTION(BlueprintPure, Category = "Character Customization")
    EChar_WeatheringLevel GetWeatheringLevel() const { return CharacterCustomization.WeatheringLevel; }

    UFUNCTION(BlueprintPure, Category = "Character Customization")
    EChar_TribalMarkings GetTribalMarkings() const { return CharacterCustomization.TribalMarkings; }

private:
    void InitializeMaterials();
    void UpdateCharacterMaterials();
    void ApplySkinToneToMaterial();
    void ApplyWeatheringToMaterial();
    void ApplyTribalMarkingsToMaterial();
    void UpdateMuscleDefinition();
    void UpdateScarring();
};