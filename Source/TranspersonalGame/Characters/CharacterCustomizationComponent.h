#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "CharacterCustomizationComponent.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean Scout"),
    Athletic    UMETA(DisplayName = "Athletic Hunter"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Heavy       UMETA(DisplayName = "Heavy Gatherer")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Pale        UMETA(DisplayName = "Pale"),
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Tan         UMETA(DisplayName = "Tan"),
    Dark        UMETA(DisplayName = "Dark"),
    VeryDark    UMETA(DisplayName = "Very Dark")
};

UENUM(BlueprintType)
enum class EChar_ClothingSet : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Loincloth"),
    Hunter      UMETA(DisplayName = "Hunter Leather"),
    Gatherer    UMETA(DisplayName = "Gatherer Fiber"),
    Warrior     UMETA(DisplayName = "Warrior Bone Armor")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CustomizationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EChar_ClothingSet ClothingSet = EChar_ClothingSet::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float BodyScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float MuscleDefinition = 1.0f;

    FChar_CustomizationData()
    {
        BodyType = EChar_BodyType::Athletic;
        SkinTone = EChar_SkinTone::Medium;
        ClothingSet = EChar_ClothingSet::Hunter;
        BodyScale = 1.0f;
        MuscleDefinition = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<UMaterialInterface*> SkinToneMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<UMaterialInterface*> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meshes")
    TArray<USkeletalMesh*> BodyTypeMeshes;

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

protected:
    UFUNCTION()
    void UpdateCharacterMesh();

    UFUNCTION()
    void UpdateMaterials();

    UFUNCTION()
    void UpdateBodyScale();

private:
    UPROPERTY()
    class USkeletalMeshComponent* CharacterMesh;

    UPROPERTY()
    class UMaterialInstanceDynamic* DynamicSkinMaterial;

    UPROPERTY()
    class UMaterialInstanceDynamic* DynamicClothingMaterial;
};

#include "CharacterCustomizationComponent.generated.h"