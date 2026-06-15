#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
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
    BasicLeather    UMETA(DisplayName = "Basic Leather"),
    FurWraps        UMETA(DisplayName = "Fur Wraps"),
    BoneArmor       UMETA(DisplayName = "Bone Armor"),
    TribalGear      UMETA(DisplayName = "Tribal Gear")
};

USTRUCT(BlueprintType)
struct FChar_CharacterCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::BasicLeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor FacePaintColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFacePaint = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleMass = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringLevel = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PrimitiveHumanCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PrimitiveHumanCharacter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Components")
    class USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Components")
    class UStaticMeshComponent* BoneToolBelt;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Components")
    class UStaticMeshComponent* SpearWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    FChar_CharacterCustomization CustomizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<class UMaterialInterface*> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<class UMaterialInterface*> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* FacePaintMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float SurvivalExperience = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float TribalRank = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float CraftingSkill = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float HuntingSkill = 30.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyCustomization(const FChar_CharacterCustomization& NewCustomization);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetClothingStyle(EChar_ClothingStyle NewStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetFacePaint(bool bEnabled, FLinearColor Color = FLinearColor::Red);

    UFUNCTION(BlueprintCallable, Category = "Character Stats")
    void AddSurvivalExperience(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Character Stats")
    void SetTribalRank(float NewRank);

    UFUNCTION(BlueprintCallable, Category = "Character Equipment")
    void EquipBoneTools();

    UFUNCTION(BlueprintCallable, Category = "Character Equipment")
    void EquipSpear();

    UFUNCTION(BlueprintPure, Category = "Character Info")
    FString GetCharacterDescription() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Character Testing")
    void TestCharacterCustomization();

protected:
    void UpdateCharacterMaterials();
    void UpdateClothing();
    void UpdateFacePaint();
    void SetupDefaultEquipment();

private:
    bool bIsCustomizationApplied = false;
    float LastCustomizationTime = 0.0f;
};