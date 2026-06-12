#include "Char_MetaHumanManager.h"
#include "TranspersonalCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

AChar_MetaHumanManager::AChar_MetaHumanManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize default character presets
    InitializeDefaultPresets();
}

void AChar_MetaHumanManager::BeginPlay()
{
    Super::BeginPlay();
}

void AChar_MetaHumanManager::InitializeDefaultPresets()
{
    // Hunter preset - weathered and scarred
    FChar_CharacterPreset HunterPreset;
    HunterPreset.PresetName = TEXT("Tribal Hunter");
    HunterPreset.SkinTone = EChar_SkinTone::Tan;
    HunterPreset.BodyBuild = EChar_BodyBuild::Muscular;
    HunterPreset.ClothingStyle = EChar_ClothingStyle::LeatherWraps;
    HunterPreset.ScarIntensity = 0.7f;
    HunterPreset.WeatheringLevel = 0.8f;
    AvailablePresets.Add(HunterPreset);

    // Gatherer preset - younger and cleaner
    FChar_CharacterPreset GathererPreset;
    GathererPreset.PresetName = TEXT("Plant Gatherer");
    GathererPreset.SkinTone = EChar_SkinTone::Medium;
    GathererPreset.BodyBuild = EChar_BodyBuild::Lean;
    GathererPreset.ClothingStyle = EChar_ClothingStyle::PlantFiber;
    GathererPreset.ScarIntensity = 0.2f;
    GathererPreset.WeatheringLevel = 0.4f;
    AvailablePresets.Add(GathererPreset);

    // Warrior preset - battle-hardened
    FChar_CharacterPreset WarriorPreset;
    WarriorPreset.PresetName = TEXT("Tribal Warrior");
    WarriorPreset.SkinTone = EChar_SkinTone::Dark;
    WarriorPreset.BodyBuild = EChar_BodyBuild::Muscular;
    WarriorPreset.ClothingStyle = EChar_ClothingStyle::AnimalHide;
    WarriorPreset.ScarIntensity = 0.9f;
    WarriorPreset.WeatheringLevel = 0.7f;
    AvailablePresets.Add(WarriorPreset);

    // Scout preset - agile and light
    FChar_CharacterPreset ScoutPreset;
    ScoutPreset.PresetName = TEXT("Tribal Scout");
    ScoutPreset.SkinTone = EChar_SkinTone::Olive;
    ScoutPreset.BodyBuild = EChar_BodyBuild::Lean;
    ScoutPreset.ClothingStyle = EChar_ClothingStyle::Minimal;
    ScoutPreset.ScarIntensity = 0.4f;
    ScoutPreset.WeatheringLevel = 0.6f;
    AvailablePresets.Add(ScoutPreset);

    // Elder preset - wise and weathered
    FChar_CharacterPreset ElderPreset;
    ElderPreset.PresetName = TEXT("Tribal Elder");
    ElderPreset.SkinTone = EChar_SkinTone::Fair;
    ElderPreset.BodyBuild = EChar_BodyBuild::Average;
    ElderPreset.ClothingStyle = EChar_ClothingStyle::FurCloak;
    ElderPreset.ScarIntensity = 0.5f;
    ElderPreset.WeatheringLevel = 0.9f;
    AvailablePresets.Add(ElderPreset);
}

void AChar_MetaHumanManager::ApplyCharacterPreset(ATranspersonalCharacter* Character, const FChar_CharacterPreset& Preset)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("AChar_MetaHumanManager::ApplyCharacterPreset - Character is null"));
        return;
    }

    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("AChar_MetaHumanManager::ApplyCharacterPreset - Character has no mesh component"));
        return;
    }

    // Apply skin tone
    SetCharacterSkinTone(Character, Preset.SkinTone);
    
    // Apply clothing style
    SetCharacterClothing(Character, Preset.ClothingStyle);
    
    // Apply weathering effects
    ApplyWeatheringEffects(Character, Preset.WeatheringLevel);

    UE_LOG(LogTemp, Log, TEXT("Applied character preset: %s"), *Preset.PresetName);
}

void AChar_MetaHumanManager::RandomizeCharacterAppearance(ATranspersonalCharacter* Character)
{
    if (!Character || AvailablePresets.Num() == 0)
    {
        return;
    }

    FChar_CharacterPreset RandomPreset = GetRandomPreset();
    
    // Add some randomization to the preset
    RandomPreset.ScarIntensity = FMath::RandRange(0.0f, 1.0f);
    RandomPreset.WeatheringLevel = FMath::RandRange(0.3f, 0.9f);
    
    ApplyCharacterPreset(Character, RandomPreset);
}

FChar_CharacterPreset AChar_MetaHumanManager::GetRandomPreset() const
{
    if (AvailablePresets.Num() == 0)
    {
        return FChar_CharacterPreset();
    }

    int32 RandomIndex = FMath::RandRange(0, AvailablePresets.Num() - 1);
    return AvailablePresets[RandomIndex];
}

void AChar_MetaHumanManager::SetCharacterSkinTone(ATranspersonalCharacter* Character, EChar_SkinTone SkinTone)
{
    if (!Character)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }

    UMaterialInterface* SkinMaterial = GetSkinMaterialForTone(SkinTone);
    if (SkinMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(SkinMaterial, this);
        if (DynamicMaterial)
        {
            MeshComp->SetMaterial(0, DynamicMaterial);
            UE_LOG(LogTemp, Log, TEXT("Applied skin tone material to character"));
        }
    }
}

void AChar_MetaHumanManager::SetCharacterClothing(ATranspersonalCharacter* Character, EChar_ClothingStyle ClothingStyle)
{
    if (!Character)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }

    UMaterialInterface* ClothingMaterial = GetClothingMaterialForStyle(ClothingStyle);
    if (ClothingMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ClothingMaterial, this);
        if (DynamicMaterial)
        {
            // Apply to clothing material slots (typically slots 1-3)
            for (int32 i = 1; i < 4; i++)
            {
                if (MeshComp->GetMaterial(i))
                {
                    MeshComp->SetMaterial(i, DynamicMaterial);
                }
            }
            UE_LOG(LogTemp, Log, TEXT("Applied clothing style material to character"));
        }
    }
}

void AChar_MetaHumanManager::ApplyWeatheringEffects(ATranspersonalCharacter* Character, float WeatheringLevel)
{
    if (!Character)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp)
    {
        return;
    }

    // Apply weathering to all material slots
    for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
    {
        UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(i);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                // Set weathering parameters if they exist
                DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), WeatheringLevel);
                DynamicMaterial->SetScalarParameterValue(TEXT("Dirt"), WeatheringLevel * 0.8f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Wear"), WeatheringLevel * 0.6f);
                
                MeshComp->SetMaterial(i, DynamicMaterial);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Applied weathering effects: %f"), WeatheringLevel);
}

UMaterialInterface* AChar_MetaHumanManager::GetSkinMaterialForTone(EChar_SkinTone SkinTone) const
{
    if (SkinMaterials.Num() == 0)
    {
        return nullptr;
    }

    int32 MaterialIndex = FMath::Clamp((int32)SkinTone, 0, SkinMaterials.Num() - 1);
    return SkinMaterials[MaterialIndex].LoadSynchronous();
}

UMaterialInterface* AChar_MetaHumanManager::GetClothingMaterialForStyle(EChar_ClothingStyle ClothingStyle) const
{
    if (ClothingMaterials.Num() == 0)
    {
        return nullptr;
    }

    int32 MaterialIndex = FMath::Clamp((int32)ClothingStyle, 0, ClothingMaterials.Num() - 1);
    return ClothingMaterials[MaterialIndex].LoadSynchronous();
}