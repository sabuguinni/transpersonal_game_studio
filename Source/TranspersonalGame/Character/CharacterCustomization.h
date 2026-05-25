#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "CharacterCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light		UMETA(DisplayName = "Light"),
    Medium		UMETA(DisplayName = "Medium"),
    Dark		UMETA(DisplayName = "Dark"),
    Weathered	UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean		UMETA(DisplayName = "Lean"),
    Athletic	UMETA(DisplayName = "Athletic"),
    Muscular	UMETA(DisplayName = "Muscular"),
    Heavy		UMETA(DisplayName = "Heavy")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    Minimal		UMETA(DisplayName = "Minimal Hide"),
    Hunter		UMETA(DisplayName = "Hunter Leather"),
    Gatherer	UMETA(DisplayName = "Gatherer Fur"),
    Tribal		UMETA(DisplayName = "Tribal Decorated")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalMarkings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBattleScars = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_CustomizationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_CustomizationComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    FChar_AppearanceData AppearanceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    class USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    TArray<class UMaterialInterface*> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Customization")
    TArray<class USkeletalMesh*> ClothingMeshes;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyAppearance(const FChar_AppearanceData& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    FChar_AppearanceData GetCurrentAppearance() const { return AppearanceData; }

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetClothingStyle(EChar_ClothingStyle NewStyle);

private:
    void UpdateCharacterMesh();
    void UpdateMaterials();
    void UpdateClothing();
};