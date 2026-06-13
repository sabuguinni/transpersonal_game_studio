#include "Char_TribalWarriorAsset.h"
#include "Engine/Engine.h"

UChar_TribalWarriorAsset::UChar_TribalWarriorAsset()
{
    // Initialize default asset paths
    WarriorMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/TribalWarrior/SK_TribalWarrior")));
    HideClothingMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Materials/Characters/M_AnimalHide")));
    BoneJewelryMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Materials/Characters/M_BoneJewelry")));
    StoneSpearMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Weapons/Primitive/SM_StoneSpear")));

    SetupDefaultColorVariations();
}

void UChar_TribalWarriorAsset::InitializeDefaultReferences()
{
    // Force load critical assets
    if (!WarriorMesh.IsNull())
    {
        WarriorMesh.LoadSynchronous();
    }

    if (!HideClothingMaterial.IsNull())
    {
        HideClothingMaterial.LoadSynchronous();
    }

    if (!BoneJewelryMaterial.IsNull())
    {
        BoneJewelryMaterial.LoadSynchronous();
    }

    if (!StoneSpearMesh.IsNull())
    {
        StoneSpearMesh.LoadSynchronous();
    }

    UE_LOG(LogTemp, Log, TEXT("TribalWarriorAsset: Default references initialized"));
}

bool UChar_TribalWarriorAsset::ValidateAssetReferences() const
{
    bool bAllValid = true;

    if (WarriorMesh.IsNull() || !WarriorMesh.Get())
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalWarriorAsset: WarriorMesh is invalid"));
        bAllValid = false;
    }

    if (HideClothingMaterial.IsNull() || !HideClothingMaterial.Get())
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalWarriorAsset: HideClothingMaterial is invalid"));
        bAllValid = false;
    }

    if (BoneJewelryMaterial.IsNull() || !BoneJewelryMaterial.Get())
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalWarriorAsset: BoneJewelryMaterial is invalid"));
        bAllValid = false;
    }

    if (StoneSpearMesh.IsNull() || !StoneSpearMesh.Get())
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalWarriorAsset: StoneSpearMesh is invalid"));
        bAllValid = false;
    }

    return bAllValid;
}

FLinearColor UChar_TribalWarriorAsset::GetRandomSkinTone() const
{
    if (SkinToneVariations.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, SkinToneVariations.Num() - 1);
        return SkinToneVariations[RandomIndex];
    }
    
    return FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Default medium skin tone
}

FLinearColor UChar_TribalWarriorAsset::GetRandomHairColor() const
{
    if (HairColorVariations.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, HairColorVariations.Num() - 1);
        return HairColorVariations[RandomIndex];
    }
    
    return FLinearColor(0.2f, 0.1f, 0.05f, 1.0f); // Default dark brown
}

void UChar_TribalWarriorAsset::SetupDefaultColorVariations()
{
    // Skin tone variations for tribal warriors
    SkinToneVariations.Empty();
    SkinToneVariations.Add(FLinearColor(0.95f, 0.8f, 0.6f, 1.0f));   // Light
    SkinToneVariations.Add(FLinearColor(0.8f, 0.6f, 0.4f, 1.0f));    // Medium
    SkinToneVariations.Add(FLinearColor(0.6f, 0.4f, 0.3f, 1.0f));    // Tan
    SkinToneVariations.Add(FLinearColor(0.4f, 0.3f, 0.2f, 1.0f));    // Dark
    SkinToneVariations.Add(FLinearColor(0.3f, 0.2f, 0.15f, 1.0f));   // Deep

    // Hair color variations
    HairColorVariations.Empty();
    HairColorVariations.Add(FLinearColor(0.1f, 0.05f, 0.02f, 1.0f)); // Black
    HairColorVariations.Add(FLinearColor(0.2f, 0.1f, 0.05f, 1.0f));  // Dark Brown
    HairColorVariations.Add(FLinearColor(0.4f, 0.2f, 0.1f, 1.0f));   // Brown
    HairColorVariations.Add(FLinearColor(0.6f, 0.4f, 0.2f, 1.0f));   // Light Brown
    HairColorVariations.Add(FLinearColor(0.8f, 0.6f, 0.3f, 1.0f));   // Blonde
    HairColorVariations.Add(FLinearColor(0.5f, 0.2f, 0.1f, 1.0f));   // Auburn
}