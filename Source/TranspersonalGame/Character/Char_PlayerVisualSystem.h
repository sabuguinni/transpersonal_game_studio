#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_PlayerVisualSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Tall        UMETA(DisplayName = "Tall")
};

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
    Short       UMETA(DisplayName = "Short"),
    Long        UMETA(DisplayName = "Long"),
    Braided     UMETA(DisplayName = "Braided"),
    Shaved      UMETA(DisplayName = "Shaved")
};

USTRUCT(BlueprintType)
struct FChar_VisualPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_HairStyle HairStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> HairMaterial;

    FChar_VisualPreset()
    {
        PresetName = TEXT("Default");
        BodyType = EChar_BodyType::Athletic;
        SkinTone = EChar_SkinTone::Medium;
        HairStyle = EChar_HairStyle::Short;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PlayerVisualSystem : public AActor
{
    GENERATED_BODY()

public:
    AChar_PlayerVisualSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* AccessoryMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Presets")
    TArray<FChar_VisualPreset> AvailablePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Appearance")
    FChar_VisualPreset CurrentPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    float HeightScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    float WidthScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FLinearColor SkinColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FLinearColor HairColor;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyVisualPreset(const FChar_VisualPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHairStyle(EChar_HairStyle NewHairStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetCharacterMesh(USkeletalMesh* NewMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinMaterial(UMaterialInterface* NewMaterial);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHairMaterial(UMaterialInterface* NewMaterial);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHeightScale(float NewScale);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetWidthScale(float NewScale);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHairColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void EquipWeapon(UStaticMesh* WeaponMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void EquipAccessory(UStaticMesh* AccessoryMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    FChar_VisualPreset GetCurrentPreset() const { return CurrentPreset; }

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    TArray<FChar_VisualPreset> GetAvailablePresets() const { return AvailablePresets; }

private:
    void InitializeDefaultPresets();
    void UpdateCharacterAppearance();
    void ApplyMaterialParameters();
    void ApplyScaling();
};