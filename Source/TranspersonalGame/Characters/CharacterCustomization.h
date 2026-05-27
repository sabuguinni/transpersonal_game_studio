#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "CharacterCustomization.generated.h"

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
    Pale        UMETA(DisplayName = "Pale"),
    Fair        UMETA(DisplayName = "Fair"),
    Olive       UMETA(DisplayName = "Olive"),
    Bronze      UMETA(DisplayName = "Bronze"),
    Dark        UMETA(DisplayName = "Dark"),
    VeryDark    UMETA(DisplayName = "Very Dark")
};

UENUM(BlueprintType)
enum class EChar_ClothingSet : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Wraps"),
    Hunter      UMETA(DisplayName = "Hunter Gear"),
    Gatherer    UMETA(DisplayName = "Gatherer Robes"),
    Shaman      UMETA(DisplayName = "Shaman Attire"),
    Warrior     UMETA(DisplayName = "Warrior Armor")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CustomizationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Bronze;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EChar_ClothingSet ClothingSet = EChar_ClothingSet::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float MuscleDefinition = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float BodyFat = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float ScarIntensity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringLevel = 0.4f;

    FChar_CustomizationData()
    {
        BodyType = EChar_BodyType::Athletic;
        SkinTone = EChar_SkinTone::Bronze;
        ClothingSet = EChar_ClothingSet::Hunter;
        MuscleDefinition = 0.5f;
        BodyFat = 0.3f;
        ScarIntensity = 0.2f;
        WeatheringLevel = 0.4f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_CustomizationAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TMap<EChar_SkinTone, TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TMap<EChar_ClothingSet, TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meshes")
    TMap<EChar_BodyType, TSoftObjectPtr<USkeletalMesh>> BodyMeshes;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterCustomizationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterCustomizationComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FChar_CustomizationData CustomizationData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    TSoftObjectPtr<UChar_CustomizationAsset> CustomizationAsset;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyCustomization();

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetClothingSet(EChar_ClothingSet NewClothingSet);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintPure, Category = "Customization")
    FChar_CustomizationData GetCustomizationData() const { return CustomizationData; }

private:
    UPROPERTY()
    class USkeletalMeshComponent* CachedMeshComponent;

    void UpdateMeshAndMaterials();
    void ApplySkinMaterial();
    void ApplyClothingMaterial();
    void UpdateBodyMorphTargets();
};