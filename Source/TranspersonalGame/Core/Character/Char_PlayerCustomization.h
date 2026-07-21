#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_PlayerCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Heavy       UMETA(DisplayName = "Heavy"),
    Agile       UMETA(DisplayName = "Agile")
};

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
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    PlantFiber      UMETA(DisplayName = "Plant Fiber"),
    Minimal         UMETA(DisplayName = "Minimal"),
    Decorated       UMETA(DisplayName = "Decorated")
};

USTRUCT(BlueprintType)
struct FChar_CustomizationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Muscular;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::AnimalHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = true;

    FChar_CustomizationData()
    {
        BodyType = EChar_BodyType::Muscular;
        SkinTone = EChar_SkinTone::Medium;
        ClothingStyle = EChar_ClothingStyle::AnimalHide;
        SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
        bHasTribalMarkings = false;
        bHasScars = true;
    }
};

/**
 * Component for customizing player character appearance in prehistoric setting
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerCustomization : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerCustomization();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FChar_CustomizationData CustomizationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<TSoftObjectPtr<USkeletalMesh>> BodyMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

public:
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyCustomization();

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetClothingStyle(EChar_ClothingStyle NewClothingStyle);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetSkinColor(FLinearColor NewSkinColor);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetClothingColor(FLinearColor NewClothingColor);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ToggleTribalMarkings(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ToggleScars(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    FChar_CustomizationData GetCustomizationData() const { return CustomizationData; }

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void LoadCustomizationData(const FChar_CustomizationData& Data);

protected:
    UFUNCTION()
    void UpdateCharacterMesh();

    UFUNCTION()
    void UpdateCharacterMaterials();

    UFUNCTION()
    USkeletalMeshComponent* GetCharacterMesh() const;
};

#include "Char_PlayerCustomization.generated.h"