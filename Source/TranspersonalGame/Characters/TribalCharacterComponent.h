#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "../SharedTypes.h"
#include "TribalCharacterComponent.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    PaleTanned      UMETA(DisplayName = "Pale Tanned"),
    MediumTanned    UMETA(DisplayName = "Medium Tanned"),
    DarkTanned      UMETA(DisplayName = "Dark Tanned"),
    WeatheredBronze UMETA(DisplayName = "Weathered Bronze"),
    DeepBronze      UMETA(DisplayName = "Deep Bronze")
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean Hunter"),
    Athletic    UMETA(DisplayName = "Athletic Survivor"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Stocky      UMETA(DisplayName = "Stocky Gatherer"),
    Weathered   UMETA(DisplayName = "Weathered Elder")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Hide"),
    Hunter      UMETA(DisplayName = "Hunter Garb"),
    Gatherer    UMETA(DisplayName = "Gatherer Wraps"),
    Warrior     UMETA(DisplayName = "Warrior Armor"),
    Shaman      UMETA(DisplayName = "Ritual Garments")
};

UENUM(BlueprintType)
enum class EChar_TribalMarking : uint8
{
    None        UMETA(DisplayName = "No Markings"),
    Dots        UMETA(DisplayName = "Dot Patterns"),
    Lines       UMETA(DisplayName = "Line Patterns"),
    Spirals     UMETA(DisplayName = "Spiral Designs"),
    Animals     UMETA(DisplayName = "Animal Symbols"),
    Handprints  UMETA(DisplayName = "Handprint Marks")
};

USTRUCT(BlueprintType)
struct FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::MediumTanned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalMarking TribalMarking = EChar_TribalMarking::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MuscleDefinition = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor TribalMarkingColor = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);

    FChar_AppearanceData()
    {
        SkinTone = EChar_SkinTone::MediumTanned;
        BodyType = EChar_BodyType::Athletic;
        ClothingStyle = EChar_ClothingStyle::Hunter;
        TribalMarking = EChar_TribalMarking::None;
        ScarIntensity = 0.3f;
        WeatheringIntensity = 0.5f;
        MuscleDefinition = 0.7f;
        TribalMarkingColor = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTribalCharacterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTribalCharacterComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_AppearanceData AppearanceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> BaseClothingMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMaterialInstanceDynamic> DynamicSkinMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMaterialInstanceDynamic> DynamicClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TObjectPtr<USkeletalMeshComponent> CharacterMesh;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyAppearance(const FChar_AppearanceData& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    FChar_AppearanceData GetAppearanceData() const { return AppearanceData; }

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetClothingStyle(EChar_ClothingStyle NewClothingStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetTribalMarking(EChar_TribalMarking NewMarking, FLinearColor MarkingColor);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void UpdateMaterialParameters();

    UFUNCTION(CallInEditor, Category = "Character Appearance")
    void PreviewRandomAppearance();

private:
    void InitializeMaterials();
    void UpdateSkinMaterial();
    void UpdateClothingMaterial();
    FLinearColor GetSkinToneColor(EChar_SkinTone SkinTone) const;
    float GetBodyTypeScale(EChar_BodyType BodyType) const;
};