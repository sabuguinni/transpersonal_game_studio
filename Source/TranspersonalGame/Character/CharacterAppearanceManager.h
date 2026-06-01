#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "CharacterAppearanceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearancePreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFacePaint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor FacePaintColor;

    FChar_AppearancePreset()
    {
        PresetName = TEXT("Default");
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        BodyScale = 1.0f;
        bHasFacePaint = false;
        FacePaintColor = FLinearColor::Red;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ClothingItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EClothingSlot ClothingSlot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float WarmthValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float ProtectionValue;

    FChar_ClothingItem()
    {
        ItemName = TEXT("Basic Hide");
        ClothingSlot = EClothingSlot::Torso;
        WarmthValue = 10.0f;
        ProtectionValue = 5.0f;
    }
};

/**
 * Manages character visual appearance, clothing, and customization
 * Handles primitive tribal aesthetics appropriate for Cretaceous survival
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterAppearanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterAppearanceManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FChar_AppearancePreset CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FChar_AppearancePreset> AvailablePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FChar_ClothingItem> EquippedClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FChar_ClothingItem> AvailableClothing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> BodyMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    bool bAllowRuntimeCustomization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    float AppearanceChangeTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ApplyAppearancePreset(const FChar_AppearancePreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetSkinTone(const FLinearColor& NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetHairColor(const FLinearColor& NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetEyeColor(const FLinearColor& NewEyeColor);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetBodyScale(float NewScale);

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    bool EquipClothingItem(const FChar_ClothingItem& ClothingItem);

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    bool UnequipClothingSlot(EClothingSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    FChar_ClothingItem GetEquippedClothing(EClothingSlot Slot) const;

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    float GetTotalWarmth() const;

    UFUNCTION(BlueprintCallable, Category = "Clothing")
    float GetTotalProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ApplyFacePaint(const FLinearColor& PaintColor);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void RemoveFacePaint();

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SaveCurrentAsPreset(const FString& PresetName);

    UFUNCTION(BlueprintPure, Category = "Appearance")
    FChar_AppearancePreset GetCurrentAppearance() const { return CurrentAppearance; }

    UFUNCTION(BlueprintPure, Category = "Clothing")
    TArray<FChar_ClothingItem> GetEquippedClothingItems() const { return EquippedClothing; }

private:
    void UpdateBodyMesh();
    void UpdateSkinMaterial();
    void UpdateClothingMeshes();
    void CreateDynamicMaterials();
    
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicSkinMaterial;

    UPROPERTY()
    TMap<EClothingSlot, UMaterialInstanceDynamic*> DynamicClothingMaterials;

    void InitializeDefaultPresets();
    void InitializeDefaultClothing();
};