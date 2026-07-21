#include "Char_TribalCharacterAsset.h"
#include "Engine/Engine.h"

UChar_TribalCharacterAsset::UChar_TribalCharacterAsset()
{
    CharacterName = TEXT("Tribal Warrior");
    TribalRole = EChar_TribalRole::Warrior;
    BaseMesh = nullptr;
    
    // Initialize default stats based on role
    BaseStats.Strength = 60.0f;
    BaseStats.Agility = 55.0f;
    BaseStats.Intelligence = 45.0f;
    BaseStats.Endurance = 65.0f;
    BaseStats.SurvivalSkill = 70.0f;

    // Asset generation notes for failed meshy_generate
    AssetGenerationNotes = TEXT("TODO_ASSET_GENERATION_FAILED: Primitive Cretaceous tribal warrior character, full body T-pose, scientifically accurate human proportions, muscular build, wearing animal hide clothing and bone jewelry, holding stone spear, neutral expression, game-ready topology for rigging. Target polycount: 25000. Art style: realistic. Generated concept art available as fallback reference.");
}

FChar_TribalStats UChar_TribalCharacterAsset::GetModifiedStats() const
{
    FChar_TribalStats ModifiedStats = BaseStats;
    
    // Apply role-based stat modifications
    switch (TribalRole)
    {
        case EChar_TribalRole::Hunter:
            ModifiedStats.Agility += 15.0f;
            ModifiedStats.SurvivalSkill += 10.0f;
            ModifiedStats.Intelligence += 5.0f;
            break;
            
        case EChar_TribalRole::Gatherer:
            ModifiedStats.Intelligence += 10.0f;
            ModifiedStats.SurvivalSkill += 15.0f;
            ModifiedStats.Endurance += 5.0f;
            break;
            
        case EChar_TribalRole::Warrior:
            ModifiedStats.Strength += 15.0f;
            ModifiedStats.Endurance += 10.0f;
            ModifiedStats.Agility += 5.0f;
            break;
            
        case EChar_TribalRole::Elder:
            ModifiedStats.Intelligence += 20.0f;
            ModifiedStats.SurvivalSkill += 15.0f;
            ModifiedStats.Strength -= 10.0f;
            ModifiedStats.Agility -= 5.0f;
            break;
            
        case EChar_TribalRole::Shaman:
            ModifiedStats.Intelligence += 25.0f;
            ModifiedStats.SurvivalSkill += 10.0f;
            ModifiedStats.Strength -= 5.0f;
            break;
    }
    
    // Clamp all stats to valid range
    ModifiedStats.Strength = FMath::Clamp(ModifiedStats.Strength, 0.0f, 100.0f);
    ModifiedStats.Agility = FMath::Clamp(ModifiedStats.Agility, 0.0f, 100.0f);
    ModifiedStats.Intelligence = FMath::Clamp(ModifiedStats.Intelligence, 0.0f, 100.0f);
    ModifiedStats.Endurance = FMath::Clamp(ModifiedStats.Endurance, 0.0f, 100.0f);
    ModifiedStats.SurvivalSkill = FMath::Clamp(ModifiedStats.SurvivalSkill, 0.0f, 100.0f);
    
    return ModifiedStats;
}

bool UChar_TribalCharacterAsset::HasValidEquipment() const
{
    return !Equipment.WeaponMesh.IsNull() || 
           !Equipment.ClothingMesh.IsNull() || 
           !Equipment.AccessoryMesh.IsNull() ||
           !Equipment.SkinMaterial.IsNull();
}

TArray<TSoftObjectPtr<UMaterialInterface>> UChar_TribalCharacterAsset::GetAllMaterials() const
{
    TArray<TSoftObjectPtr<UMaterialInterface>> AllMaterials;
    
    // Add equipment material
    if (!Equipment.SkinMaterial.IsNull())
    {
        AllMaterials.Add(Equipment.SkinMaterial);
    }
    
    // Add skin variations
    for (const auto& SkinMaterial : SkinVariations)
    {
        if (!SkinMaterial.IsNull())
        {
            AllMaterials.Add(SkinMaterial);
        }
    }
    
    return AllMaterials;
}