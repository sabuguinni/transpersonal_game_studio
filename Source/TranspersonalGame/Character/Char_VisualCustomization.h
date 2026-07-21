#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_VisualCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Tanned      UMETA(DisplayName = "Tanned")
};

UENUM(BlueprintType)
enum class EChar_HairStyle : uint8
{
    Long        UMETA(DisplayName = "Long"),
    Braided     UMETA(DisplayName = "Braided"),
    Short       UMETA(DisplayName = "Short"),
    Shaved      UMETA(DisplayName = "Shaved")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    Leather     UMETA(DisplayName = "Leather"),
    Fur         UMETA(DisplayName = "Fur"),
    Woven       UMETA(DisplayName = "Woven"),
    Minimal     UMETA(DisplayName = "Minimal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_VisualPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    EChar_HairStyle HairStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    EChar_ClothingStyle ClothingStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor FacePaintColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    FChar_VisualPreset()
    {
        PresetName = TEXT("Default");
        SkinTone = EChar_SkinTone::Medium;
        HairStyle = EChar_HairStyle::Long;
        ClothingStyle = EChar_ClothingStyle::Leather;
        FacePaintColor = FLinearColor::Red;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_VisualCustomization : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_VisualCustomization();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visual")
    FChar_VisualPreset CurrentPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visual")
    TArray<FChar_VisualPreset> AvailablePresets;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Visual")
    class USkeletalMeshComponent* CharacterMesh;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void ApplyVisualPreset(const FChar_VisualPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetHairStyle(EChar_HairStyle NewHairStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetClothingStyle(EChar_ClothingStyle NewClothingStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetFacePaintColor(FLinearColor NewColor);

    UFUNCTION(BlueprintPure, Category = "Character Visual")
    FChar_VisualPreset GetCurrentPreset() const { return CurrentPreset; }

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void InitializeDefaultPresets();

private:
    void UpdateCharacterAppearance();
    void LoadCharacterMesh(TSoftObjectPtr<USkeletalMesh> MeshAsset);
    void ApplyMaterialParameters();
};