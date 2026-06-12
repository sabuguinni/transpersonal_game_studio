#include "Char_TribalWarriorAsset.h"

UChar_TribalWarriorAsset::UChar_TribalWarriorAsset()
{
    // Default character appearance settings
    HeightScale = 1.0f;
    SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Natural skin tone
    HairColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f); // Dark brown hair
    
    // Default behavior settings
    AggressionLevel = 0.5f;
    SocialRank = 0.5f;
    
    // Default character tags
    CharacterTags.Add(FName("Tribal"));
    CharacterTags.Add(FName("Warrior"));
    CharacterTags.Add(FName("Human"));
}

bool UChar_TribalWarriorAsset::IsAssetValid() const
{
    // Check if essential assets are assigned
    bool bHasBodyMesh = !BodyMesh.IsNull();
    bool bHasSkinMaterial = !SkinMaterial.IsNull();
    bool bHasWeapon = !SpearMesh.IsNull() || !KnifeMesh.IsNull();
    
    return bHasBodyMesh && bHasSkinMaterial && bHasWeapon;
}

TArray<FString> UChar_TribalWarriorAsset::GetMissingAssets() const
{
    TArray<FString> MissingAssets;
    
    if (BodyMesh.IsNull())
    {
        MissingAssets.Add(TEXT("Body Mesh"));
    }
    
    if (SkinMaterial.IsNull())
    {
        MissingAssets.Add(TEXT("Skin Material"));
    }
    
    if (ClothingMaterial.IsNull())
    {
        MissingAssets.Add(TEXT("Clothing Material"));
    }
    
    if (SpearMesh.IsNull())
    {
        MissingAssets.Add(TEXT("Spear Mesh"));
    }
    
    if (KnifeMesh.IsNull())
    {
        MissingAssets.Add(TEXT("Knife Mesh"));
    }
    
    if (BoneNecklaceMesh.IsNull())
    {
        MissingAssets.Add(TEXT("Bone Necklace Mesh"));
    }
    
    return MissingAssets;
}