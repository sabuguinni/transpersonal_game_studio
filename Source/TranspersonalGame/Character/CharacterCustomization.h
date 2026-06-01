#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "CharacterCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Average     UMETA(DisplayName = "Average"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Heavy       UMETA(DisplayName = "Heavy")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Tanned      UMETA(DisplayName = "Tanned"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_HairStyle : uint8
{
    Short       UMETA(DisplayName = "Short"),
    Long        UMETA(DisplayName = "Long"),
    Braided     UMETA(DisplayName = "Braided"),
    Shaved      UMETA(DisplayName = "Shaved"),
    Messy       UMETA(DisplayName = "Messy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CustomizationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Average;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_HairStyle HairStyle = EChar_HairStyle::Short;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor::Brown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName;

    FChar_CustomizationData()
    {
        BodyType = EChar_BodyType::Average;
        SkinTone = EChar_SkinTone::Medium;
        HairStyle = EChar_HairStyle::Short;
        HairColor = FLinearColor::Black;
        EyeColor = FLinearColor::Brown;
        Height = 1.0f;
        Weight = 1.0f;
        CharacterName = TEXT("Survivor");
    }
};

UCLASS(BlueprintType, Blueprintable)
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<TSoftObjectPtr<USkeletalMesh>> BodyMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> HairMaterials;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyCustomization();

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetHairStyle(EChar_HairStyle NewHairStyle);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetHairColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetEyeColor(FLinearColor NewColor);

private:
    UPROPERTY()
    class ATranspersonalCharacter* OwnerCharacter;

    void UpdateMeshAndMaterials();
    USkeletalMesh* GetMeshForBodyType(EChar_BodyType BodyType);
    UMaterialInterface* GetMaterialForSkinTone(EChar_SkinTone SkinTone);
    UMaterialInterface* GetMaterialForHair(EChar_HairStyle HairStyle);
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterCustomizationPreset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FChar_CustomizationData PresetData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    TSoftObjectPtr<UTexture2D> PreviewImage;
};