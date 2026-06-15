#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Char_PrimitiveHumanCharacter.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"), 
    Dark        UMETA(DisplayName = "Dark"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    BasicHide   UMETA(DisplayName = "Basic Hide"),
    FurTrim     UMETA(DisplayName = "Fur Trim"),
    BoneArmor   UMETA(DisplayName = "Bone Armor"),
    Tribal      UMETA(DisplayName = "Tribal")
};

USTRUCT(BlueprintType)
struct FChar_CharacterCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::BasicHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyMassIndex = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBoneJewelry = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = false;
};

UCLASS(Blueprintable, BlueprintType)
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

    // Weapon components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
    UStaticMeshComponent* StoneAxeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
    UStaticMeshComponent* SpearComponent;

    // Clothing/Armor components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothing")
    UStaticMeshComponent* HideClothingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clothing")
    UStaticMeshComponent* BoneJewelryComponent;

    // Character stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float TribalRank = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float SurvivalExperience = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float CraftingSkill = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float HuntingSkill = 1.0f;

public:
    // Customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyCharacterCustomization(const FChar_CharacterCustomization& NewCustomization);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetClothingStyle(EChar_ClothingStyle NewClothingStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ToggleTribalMarkings(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ToggleBoneJewelry(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ToggleScars(bool bEnabled);

    // Equipment functions
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipStoneAxe(bool bEquipped);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipSpear(bool bEquipped);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    FString GetCharacterDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Character")
    float GetOverallSkillLevel() const;

private:
    void UpdateMeshMaterials();
    void UpdateClothingMesh();
    void UpdateJewelryVisibility();
};