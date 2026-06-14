#include "Char_TribalWarriorAsset.h"

UChar_TribalWarriorAsset::UChar_TribalWarriorAsset()
{
    CharacterName = TEXT("Tribal Warrior");
    Rank = EChar_TribalRank::Warrior;
    Biography = FText::FromString(TEXT("A skilled warrior of the Cretaceous tribes, hardened by survival in a world of giants."));
    
    BaseMesh = nullptr;
    SkinMaterial = nullptr;
    ClothingMaterial = nullptr;
    MetalMaterial = nullptr;
    
    // Default appearance
    Appearance.SkinTone = EChar_SkinTone::Medium;
    Appearance.BodyBuild = EChar_BodyBuild::Athletic;
    Appearance.WarPaintColor = FLinearColor::Red;
    Appearance.bHasWarPaint = true;
    Appearance.bHasScarring = false;
    Appearance.MuscleMass = 1.0f;
    
    // Default stats
    BaseHealth = 100.0f;
    BaseStamina = 100.0f;
    MovementSpeed = 400.0f;
    AttackDamage = 25.0f;
    DefenseRating = 15.0f;
}

FString UChar_TribalWarriorAsset::GetCharacterDescription() const
{
    FString Description = FString::Printf(TEXT("%s - %s"), 
        *CharacterName, 
        *UEnum::GetValueAsString(Rank));
    
    // Add appearance details
    FString SkinToneStr = UEnum::GetValueAsString(Appearance.SkinTone);
    FString BodyBuildStr = UEnum::GetValueAsString(Appearance.BodyBuild);
    
    Description += FString::Printf(TEXT("\nAppearance: %s skin, %s build"), 
        *SkinToneStr, *BodyBuildStr);
    
    if (Appearance.bHasWarPaint)
    {
        Description += TEXT(", war paint");
    }
    
    if (Appearance.bHasScarring)
    {
        Description += TEXT(", battle scars");
    }
    
    // Add equipment summary
    int32 EquippedItems = 0;
    if (Equipment.WeaponMesh.IsValid()) EquippedItems++;
    if (Equipment.ShieldMesh.IsValid()) EquippedItems++;
    if (Equipment.ClothingMesh.IsValid()) EquippedItems++;
    if (Equipment.JewelryMesh.IsValid()) EquippedItems++;
    if (Equipment.HeadgearMesh.IsValid()) EquippedItems++;
    if (Equipment.FootwearMesh.IsValid()) EquippedItems++;
    
    Description += FString::Printf(TEXT("\nEquipment: %d/6 slots equipped"), EquippedItems);
    
    // Add stats summary
    Description += FString::Printf(TEXT("\nStats: HP %.0f, Stamina %.0f, Speed %.0f"), 
        BaseHealth, BaseStamina, MovementSpeed);
    
    Description += FString::Printf(TEXT("\nCombat: Attack %.0f, Defense %.0f"), 
        AttackDamage, DefenseRating);
    
    return Description;
}

bool UChar_TribalWarriorAsset::IsValidConfiguration() const
{
    // Check if we have minimum required assets
    if (!BaseMesh.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalWarriorAsset %s: Missing base mesh"), *CharacterName);
        return false;
    }
    
    if (!SkinMaterial.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalWarriorAsset %s: Missing skin material"), *CharacterName);
        return false;
    }
    
    // Check stats are reasonable
    if (BaseHealth <= 0.0f || BaseStamina <= 0.0f || MovementSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalWarriorAsset %s: Invalid stats"), *CharacterName);
        return false;
    }
    
    // Check appearance values are in valid ranges
    if (Appearance.MuscleMass < 0.1f || Appearance.MuscleMass > 2.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalWarriorAsset %s: Invalid muscle mass %.2f"), 
            *CharacterName, Appearance.MuscleMass);
        return false;
    }
    
    return true;
}